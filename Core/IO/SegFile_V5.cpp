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
#include "ReadOnlyFilter.h"
#include "FetchRawData.h"
#include "ReadOnlyChannelExtension.h"
#include "ReadOnlySegmentationExtension.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/TemporalStorage.h>
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
SegFile_V5::SegFile_V5()
: m_fetchBehaviour{new FetchRawData()}
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::load(QuaZip&          zip,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler)
{
  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  TemporalStorageSPtr storage{new TemporalStorage(tmpDir)};

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
    QString file = zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE
     && file != CLASSIFICATION_FILE
     && file != CONTENT_FILE
     && file != RELATIONS_FILE)
    {
      storage->saveSnapshot(SnapshotData(file, readCurrentFileFromZip(zip, handler)));
    }

    hasFile = zip.goToNextFile();
  }

  loadContent(analysis, zip, storage, factory, handler);

  loadRelations(analysis, zip, handler);

  return analysis;
}

//-----------------------------------------------------------------------------
void SegFile_V5::save(AnalysisPtr analysis, QuaZip& zip, ErrorHandlerSPtr handler)
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

  for(auto v : analysis->content()->vertices())
  {
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.get());
    for(auto data : item->snapshot())
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

//   foreach(ExtensionProviderSPtr provider, analysis->extensionProviders()) {
//     foreach(SnapshotData data, provider->saveSnapshot())
//     {
//       try
//       {
//         addFileToZip(data.first, data.second, zip, handler);
//       } catch (IO_Error_Exception e)
//       {
//         throw (e);
//       }
//     }
//   }
}

struct Vertex_Not_Found_Exception{};

//-----------------------------------------------------------------------------
PersistentSPtr SegFile_V5::findVertex(DirectedGraph::Vertices vertices, Persistent::Uuid uuid)
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
                                    TemporalStorageSPtr storage,
                                    CoreFactorySPtr         factory,
                                    ErrorHandlerSPtr         handler)
{
  SampleSPtr sample = factory->createSample();

  sample->setName(roVertex->name());
  sample->setUuid(roVertex->uuid());
  sample->restoreState(roVertex->state());
  sample->setStorage(storage);

  analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V5::createFilter(DirectedGraph::Vertex   roVertex,
                                    DirectedGraphSPtr       content,
                                    DirectedGraph::Vertices loadedVertices,
                                    TemporalStorageSPtr storage,
                                    CoreFactorySPtr         factory,
                                    ErrorHandlerSPtr         handler)
{
  DirectedGraph::Edges inputConections = content->inEdges(roVertex);

  OutputSList inputs;
  for(auto edge : inputConections)
  {
    DirectedGraph::Vertex input = findVertex(loadedVertices, edge.source->uuid());

    FilterSPtr filter = std::dynamic_pointer_cast<Filter>(input);
    Output::Id id     = atoi(edge.relationship.c_str());

    inputs << filter->output(id);
  }

  FilterSPtr filter;
  try
  {
    filter = factory->createFilter(inputs, roVertex->name());
  } catch (CoreFactory::Unknown_Type_Exception e)
  {
    filter = FilterSPtr{new ReadOnlyFilter(inputs, roVertex->name())};
    filter->setFetchBehaviour(m_fetchBehaviour);
  }
  filter->setErrorHandler(handler);
  filter->setName(roVertex->name());
  filter->setUuid(roVertex->uuid());
  filter->restoreState(roVertex->state());
  filter->setStorage(storage);

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V5::findOutput(DirectedGraph::Vertex   roVertex,
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
                                      TemporalStorageSPtr     storage,
                                      CoreFactorySPtr         factory,
                                      ErrorHandlerSPtr        handler)
{
  auto output = findOutput(roVertex, content, loadedVertices);

  ChannelSPtr channel = factory->createChannel(output.first, output.second);

  channel->setName(roVertex->name());
  channel->setUuid(roVertex->uuid());
  channel->restoreState(roVertex->state());
  channel->setStorage(storage);

  loadExtensions(channel, factory);

  analysis->add(channel);

  return channel;
}

//-----------------------------------------------------------------------------
QString SegFile_V5::parseCategoryName(const State& state)
{
  QStringList params = state.split(";");

  return params[2].split("=")[1];
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V5::createSegmentation(DirectedGraph::Vertex   roVertex,
                                                AnalysisSPtr            analysis,
                                                DirectedGraphSPtr       content,
                                                DirectedGraph::Vertices loadedVertices,
                                                TemporalStorageSPtr     storage,
                                                CoreFactorySPtr         factory,
                                                ErrorHandlerSPtr        handler)
{
  auto output = findOutput(roVertex, content, loadedVertices);

  SegmentationSPtr segmentation = factory->createSegmentation(output.first, output.second);

  State roState = roVertex->state();
  segmentation->setName(roVertex->name());
  segmentation->setUuid(roVertex->uuid());
  segmentation->restoreState(roState);
  segmentation->setStorage(storage);

  auto categoryName = parseCategoryName(roState);

  if (!categoryName.isEmpty())
  {
    auto category = analysis->classification()->node(categoryName);

    segmentation->setCategory(category);
  }

  analysis->add(segmentation);

  return segmentation;
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadContent(AnalysisSPtr        analysis,
                             QuaZip&             zip,
                             TemporalStorageSPtr storage,
                             CoreFactorySPtr     factory,
                             ErrorHandlerSPtr    handler)
{
  DirectedGraphSPtr content(new DirectedGraph());

  QTextStream textStream(readFileFromZip(CONTENT_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, content);

  DirectedGraph::Vertices loadedVertices;

  for(DirectedGraph::Vertex roVertex : content->vertices())
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
void SegFile_V5::loadRelations(AnalysisSPtr     analysis,
                               QuaZip&          zip,
                               ErrorHandlerSPtr handler)
{
  DirectedGraphSPtr relations(new DirectedGraph());

  QTextStream textStream(readFileFromZip(RELATIONS_FILE, zip, handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, relations);

  DirectedGraph::Vertices loadedVertices = analysis->content()->vertices();

  for(auto edge : relations->edges())
  {
    PersistentSPtr source = findVertex(loadedVertices, edge.source->uuid());
    PersistentSPtr target = findVertex(loadedVertices, edge.target->uuid());

    analysis->addRelation(source, target, edge.relationship.c_str());
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadExtensions(ChannelSPtr channel, CoreFactorySPtr factory)
{
  QString xmlFile = extensionFile(channel);

  QByteArray extenions = channel->storage()->snapshot(xmlFile);

  QXmlStreamReader xml(extenions);

  qDebug() << "Looking for" << channel->name() << "extensions:";
  while (!xml.atEnd())
  {
    xml.readNextStartElement();
    if (xml.isStartElement() && xml.name() != "Channel")
    {
      QString type = xml.name().toString();
      qDebug() << " - " << type << " found";
      State state = xml.readElementText();
      qDebug() << " * State: \n" << state;
      ChannelExtensionSPtr extension;
      try
      {
        extension = factory->createChannelExtension(type, state);
      } catch (...)
      {
        extension = ChannelExtensionSPtr{new ReadOnlyChannelExtension(type, state)};
      }
      Q_ASSERT(extension);
      channel->addExtension(extension);
    }
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::loadExtensions(SegmentationSPtr segmentation, CoreFactorySPtr factory)
{
  QString xmlFile = QString("Extensions/%1.xml").arg(segmentation->uuid());
  QByteArray extenions = segmentation->storage()->snapshot(xmlFile);

  QXmlStreamReader xml(extenions);

  qDebug() << "Looking for" << segmentation->name() << "extensions:";
  while (!xml.atEnd())
  {
    xml.readNextStartElement();
    if (xml.isStartElement())
    {
      QString type = xml.name().toString();
      qDebug() << " - " << type << " found";
      SegmentationExtensionSPtr extension;
      try
      {
        extension = factory->createSegmentationExtension(type);
      } catch (...)
      {
        extension = SegmentationExtensionSPtr{new ReadOnlySegmentationExtension(type)};
      }
      Q_ASSERT(extension);
      segmentation->addExtension(extension);
    }
  }
}
