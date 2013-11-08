/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "SegFile_V5.h"

#include <EspinaConfig.h>

#include "SegFile.h"
#include "ClassificationXML.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Extensions/ExtensionProvider.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Storage.h>
#include <Core/Factory/CoreFactory.h>


using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;
using namespace EspINA::IO::Graph;

const QString SegFile::SegFile_V5::FORMAT_INFO_FILE = "formatInfo.ini";

const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";
const QString CLASSIFICATION_FILE = "classification.xml";
const QString SEG_FILE_VERSION    = "5";

//-----------------------------------------------------------------------------
QByteArray formatInfo()
{
  QByteArray info;

  QTextStream infoStream(&info);

  infoStream << QString("SegFile Version=%1").arg(SEG_FILE_VERSION) << endl;
  infoStream << QString("EspINA Version=%1").arg(ESPINA_VERSION) << endl;

  return info;
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::load(QuaZip&         zip,
                              CoreFactorySPtr factory,
                              ErrorHandlerPtr handler)
{
  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  Persistent::StorageSPtr storage{new Persistent::Storage(tmpDir)};

  AnalysisSPtr analysis{new Analysis()};

  if (!zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (handler)
      handler->error(QObject::tr("Could not load analysis classification"));

    throw (Classification_Not_Found_Exception());
  }

  try
  {
    analysis->setClassification(ClassificationXML::parse(readCurrentFileFromZip(zip, handler)));
  } catch (ClassificationXML::Parse_Exception e)
  {
    if (handler)
      handler->error(QObject::tr("Error while loading classification"));

    throw (Parse_Exception());
  }

  bool hasFile = zip.goToFirstFile();
  while (hasFile)
  {
    QFileInfo file = zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE
     && file != CLASSIFICATION_FILE
     && file != CONTENT_FILE
     && file != RELATIONS_FILE)
    {
      storage->saveSnapshot(SnapshotData(file.fileName(), readCurrentFileFromZip(zip, handler)));
    }

    hasFile = zip.goToNextFile();
  }

  loadContent(analysis, zip, storage, factory, handler);

  loadRelations(analysis, zip, handler);

  return analysis;
}

//-----------------------------------------------------------------------------
void SegFile_V5::save(AnalysisPtr analysis, QuaZip& zip, ErrorHandlerPtr handler)
{
  try {
    addFileToZip(FORMAT_INFO_FILE, formatInfo(), zip, handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Format Information");

    throw (e);
  }

  QByteArray classification;
  try {
    classification = ClassificationXML::dump(analysis->classification(), handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Classification");

    throw(e);
  }

  try {
    addFileToZip(CLASSIFICATION_FILE, classification, zip, handler);
  }
  catch (IO_Error_Exception e){
    if (handler)
      handler->error("Error while saving Analysis Classification");

    throw(e);
  }

  std::ostringstream content;
  write(analysis->content(), content);
  try {
    addFileToZip(CONTENT_FILE, content.str().c_str(), zip, handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    throw (e);
  }

  std::ostringstream relations;
  write(analysis->relationships(), relations);
  try {
    addFileToZip(RELATIONS_FILE, relations.str().c_str(), zip, handler);
  } catch (IO_Error_Exception e)
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    throw (e);
  }

  foreach(DirectedGraph::Vertex v, analysis->content()->vertices()) {
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.get());
    foreach(SnapshotData data, item->saveSnapshot())
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      } catch (IO_Error_Exception e)
      {
        throw (e);
      }
    }
  }

  foreach(ExtensionProviderSPtr provider, analysis->extensionProviders()) {
    foreach(SnapshotData data, provider->saveSnapshot())
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      } catch (IO_Error_Exception e)
      {
        throw (e);
      }
    }
  }
}

struct Vertex_Not_Found_Exception{};

//-----------------------------------------------------------------------------
PersistentSPtr findVertex(DirectedGraph::Vertices vertices, Persistent::Uuid uuid)
{
  foreach(DirectedGraph::Vertex vertex, vertices)
  {
    if (vertex->uuid() == uuid) return vertex;
  }

  throw Vertex_Not_Found_Exception();
}

//-----------------------------------------------------------------------------
SampleSPtr SegFile_V5::createSample(DirectedGraph::Vertex   roVertex,
                                    AnalysisSPtr            analysis,
                                    Persistent::StorageSPtr storage,
                                    CoreFactorySPtr         factory,
                                    ErrorHandlerPtr         handler)
{
  SampleSPtr sample = factory->createSample();

  sample->setName(roVertex->name());
  sample->setUuid(roVertex->uuid());
  sample->restoreState(roVertex->saveState());
  sample->setPersistentStorage(storage);

  analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V5::createFilter(DirectedGraph::Vertex   roVertex,
                                    DirectedGraphSPtr       content,
                                    DirectedGraph::Vertices loadedVertices,
                                    Persistent::StorageSPtr storage,
                                    CoreFactorySPtr         factory,
                                    ErrorHandlerPtr         handler)
{
  DirectedGraph::Edges inputConections = content->inEdges(roVertex);

  OutputSList inputs;
  foreach(DirectedGraph::Edge edge, inputConections)
  {
    DirectedGraph::Vertex input = findVertex(loadedVertices, edge.source->uuid());

    FilterSPtr filter = std::dynamic_pointer_cast<Filter>(input);
    Output::Id id     = atoi(edge.relationship.c_str());

    inputs << filter->output(id);
  }

  FilterSPtr filter = factory->createFilter(inputs, roVertex->name());
  filter->setName(roVertex->name());
  filter->setUuid(roVertex->uuid());
  filter->restoreState(roVertex->saveState());
  filter->setPersistentStorage(storage);

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> findOutput(DirectedGraph::Vertex   roVertex,
                                         DirectedGraphSPtr       content,
                                         DirectedGraph::Vertices loadedVertices)
{
  QPair<FilterSPtr, Output::Id> output;

  DirectedGraph::Edges inputConections = content->inEdges(roVertex);
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  PersistentSPtr input = findVertex(loadedVertices, edge.source->uuid());

  output.first  = std::dynamic_pointer_cast<Filter>(input);
  output.second = atoi(edge.relationship.c_str());

  return output;
}
//-----------------------------------------------------------------------------
ChannelSPtr SegFile_V5::createChannel(DirectedGraph::Vertex   roVertex,
                                      AnalysisSPtr            analysis,
                                      DirectedGraphSPtr       content,
                                      DirectedGraph::Vertices loadedVertices,
                                      Persistent::StorageSPtr storage,
                                      CoreFactorySPtr         factory,
                                      ErrorHandlerPtr         handler)
{
  auto output = findOutput(roVertex, content, loadedVertices);

  ChannelSPtr channel = factory->createChannel(output.first, output.second);

  channel->setName(roVertex->name());
  channel->setUuid(roVertex->uuid());
  channel->restoreState(roVertex->saveState());
  channel->setPersistentStorage(storage);

  analysis->add(channel);

  return channel;
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V5::createSegmentation(DirectedGraph::Vertex   roVertex,
                                                AnalysisSPtr            analysis,
                                                DirectedGraphSPtr       content,
                                                DirectedGraph::Vertices loadedVertices,
                                                Persistent::StorageSPtr storage,
                                                CoreFactorySPtr         factory,
                                                ErrorHandlerPtr         handler)
{
  auto output = findOutput(roVertex, content, loadedVertices);

  SegmentationSPtr segmentation = factory->createSegmentation(output.first, output.second);

  segmentation->setName(roVertex->name());
  segmentation->setUuid(roVertex->uuid());
  segmentation->restoreState(roVertex->saveState());
  segmentation->setPersistentStorage(storage);

  analysis->add(segmentation);

  return segmentation;
}

//-----------------------------------------------------------------------------
void SegFile_V5::createExtensionProvider(DirectedGraph::Vertex   roVertex,
                                         AnalysisSPtr            analysis,
                                         Persistent::StorageSPtr storage,
                                         CoreFactorySPtr         factory,
                                         ErrorHandlerPtr         handler)
{
//   ExtensionProviderSPtr provider = factory->createExtensionProvider(roVertex->name());
// 
//   analysis->add(provider);
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadContent(AnalysisSPtr            analysis,
                             QuaZip&                 zip,
                             Persistent::StorageSPtr storage,
                             CoreFactorySPtr         factory,
                             ErrorHandlerPtr         handler)
{
  DirectedGraphSPtr content(new DirectedGraph());

  QTextStream textStream(readFileFromZip(CONTENT_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, content);

  DirectedGraph::Vertices loadedVertices;

  foreach(DirectedGraph::Vertex roVertex, content->vertices())
  {
    ReadOnlyVertex *rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());

    PersistentSPtr vertex;
    switch(rov->type())
    {
      case VertexType::SAMPLE:
      {
        vertex = createSample(roVertex, analysis, storage, factory, handler);
        break;
      }
      case VertexType::CHANNEL:
      {
        vertex = createChannel(roVertex, analysis, content, loadedVertices, storage, factory, handler);
        break;
      }
      case VertexType::FILTER:
      {
        vertex = createFilter(roVertex, content, loadedVertices, storage, factory, handler);
        break;
      }
      case VertexType::SEGMENTATION:
      {
        vertex = createSegmentation(roVertex, analysis, content, loadedVertices, storage, factory, handler);
        break;
      }
      case VertexType::EXTENSION_PROVIDER:
        createExtensionProvider(roVertex, analysis, storage, factory, handler);
        break;
      default:
        throw Graph::Unknown_Type_Found();
        break;
    }

    if (vertex != nullptr)
    {
      loadedVertices << vertex;
    }
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadRelations(AnalysisSPtr    analysis,
                               QuaZip&         zip,
                               ErrorHandlerPtr handler)
{
  DirectedGraphSPtr relations(new DirectedGraph());

  QTextStream textStream(readFileFromZip(RELATIONS_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, relations);

  DirectedGraph::Vertices loadedVertices = analysis->content()->vertices();

  foreach(DirectedGraph::Edge edge, relations->edges())
  {
    PersistentSPtr source = findVertex(loadedVertices, edge.source->uuid());
    PersistentSPtr target = findVertex(loadedVertices, edge.target->uuid());
    analysis->addRelation(source, target, edge.relationship.c_str());
  }
}


