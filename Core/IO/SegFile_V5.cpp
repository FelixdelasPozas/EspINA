/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SegFile_V5.h"
#include "SegFile.h"
#include "ClassificationXML.h"
#include "ReadOnlyFilter.h"
#include "ReadOnlyChannelExtension.h"
#include "ReadOnlySegmentationExtension.h"
#include <EspinaConfig.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/IO/FetchBehaviour/FetchRawData.h>

using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;
using namespace ESPINA::IO::Graph;

const QString SegFile::SegFile_V5::FORMAT_INFO_FILE = "formatInfo.ini";

const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";
const QString CLASSIFICATION_FILE = "classification.xml";
const QString SEG_FILE_VERSION    = "5";

struct Vertex_Not_Found_Exception{};

struct Invalid_Input_Exception{};

//-----------------------------------------------------------------------------
QByteArray formatInfo()
{
  QByteArray info;

  QTextStream infoStream(&info);

  infoStream << QString("SegFile Version=%1").arg(SEG_FILE_VERSION) << endl;
  infoStream << QString("ESPINA Version=%1").arg(ESPINA_VERSION) << endl;

  return info;
}

//-----------------------------------------------------------------------------
SegFile_V5::Loader::Loader(QuaZip &zip, CoreFactorySPtr factory, ErrorHandlerSPtr handler)
: m_zip           (zip)
, m_factory       {factory}
, m_handler       {handler}
, m_analysis      {new Analysis()}
, m_fetchBehaviour{new FetchRawData()}
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::Loader::load()
{
  m_storage = std::make_shared<TemporalStorage>();

  if (!m_zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (m_handler)
      m_handler->error(QObject::tr("Could not load analysis classification"));

    throw(Classification_Not_Found_Exception());
  }

  try
  {
    auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
    m_analysis->setClassification(ClassificationXML::parse(currentFile, m_handler));
  }
  catch (const ClassificationXML::Parse_Exception &e)
  {
    if (m_handler)
      m_handler->error(QObject::tr("Error while loading classification"));

    throw(Parse_Exception());
  }

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    QString file = m_zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE &&
      file != CLASSIFICATION_FILE &&
      file != CONTENT_FILE &&
      file != RELATIONS_FILE)
    {
      auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
      m_storage->saveSnapshot(SnapshotData(file, currentFile));
    }

    hasFile = m_zip.goToNextFile();
  }

  loadContent();

  loadRelations();

  m_analysis->setStorage(m_storage);

  return m_analysis;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V5::Loader::findVertex(DirectedGraph::Vertices vertices, Persistent::Uuid uuid)
{
  for (auto vertex : vertices)
  {
    if (vertex->uuid() == uuid)
      return vertex;
  }

  return DirectedGraph::Vertex();
}

//-----------------------------------------------------------------------------
SampleSPtr SegFile_V5::Loader::createSample(DirectedGraph::Vertex roVertex)
{
  SampleSPtr sample = m_factory->createSample();

  sample->setName(roVertex->name());
  sample->setUuid(roVertex->uuid());
  sample->restoreState(roVertex->state());
  sample->setStorage(m_storage);

  m_analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V5::Loader::createFilter(DirectedGraph::Vertex roVertex)
{
  DirectedGraph::Edges inputConections = m_content->inEdges(roVertex);

  InputSList inputs;
  int lastOutput = -1; // Debug
  for (auto edge : inputConections)
  {
    auto input  = inflateVertex(edge.source);

    auto filter = std::dynamic_pointer_cast<Filter>(input);
    auto ids    = QString(edge.relationship.c_str()).split("-");

    int inputNumber = ids[0].toInt();
    Output::Id id   = inputNumber;

    // 2014-11-06 There are some seg files created during development of v5 standard
    // and have only 1 value corresponding to the input id.
    // Those files won't work if any merge was saved.
    if (ids.size() == 2)
    {
      id = ids[1].toInt();
    }

    if (inputNumber <= lastOutput) {
      qWarning() << "Unordered outputs";
    }
    lastOutput = inputNumber;

    inputs << getInput(filter, id);
  }

  FilterSPtr filter;
  try
  {
    filter = m_factory->createFilter(inputs, roVertex->name());
  }
  catch (const CoreFactory::Unknown_Type_Exception &e)
  {
    filter = std::make_shared<ReadOnlyFilter>(inputs, roVertex->name());
    filter->setDataFactory(m_fetchBehaviour);
  }
  filter->setErrorHandler(m_handler);
  filter->setName(roVertex->name());
  filter->setUuid(roVertex->uuid());
  filter->restoreState(roVertex->state());
  filter->setStorage(m_storage);
  filter->restorePreviousOutputs();

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V5::Loader::findOutput(DirectedGraph::Vertex roVertex)
{
  QPair<FilterSPtr, Output::Id> output;

  DirectedGraph::Edges inputConections = m_content->inEdges(roVertex);
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  auto input = inflateVertex(edge.source);

  output.first = std::dynamic_pointer_cast<Filter>(input);
  output.second = atoi(edge.relationship.c_str());

  return output;
}

//-----------------------------------------------------------------------------
ChannelSPtr SegFile_V5::Loader::createChannel(DirectedGraph::Vertex roVertex)
{
  auto roOutput = findOutput(roVertex);

  auto filter   = roOutput.first;
  auto outputId = roOutput.second;

  //filter->update(outputId); // No tiene que hacer falta ya que ahora todos los filtros
                              // van a restaurar su outputs

  ChannelSPtr channel = m_factory->createChannel(filter, outputId);

  channel->setName(roVertex->name());
  channel->setUuid(roVertex->uuid());
  channel->restoreState(roVertex->state());
  channel->setStorage(m_storage);

  loadExtensions(channel);

  m_analysis->add(channel);

  return channel;
}

//-----------------------------------------------------------------------------
QString SegFile_V5::Loader::parseCategoryName(const State& state)
{
  QStringList params = state.split(";");

  return params[2].split("=")[1];
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V5::Loader::createSegmentation(DirectedGraph::Vertex roVertex)
{
  auto roOutput = findOutput(roVertex);

  auto filter   = roOutput.first;
  auto outputId = roOutput.second;

  if (!filter)
  {
    throw Invalid_Input_Exception();
  }

  auto segmentation = m_factory->createSegmentation(filter, outputId);

  State roState = roVertex->state();
  segmentation->setName(roVertex->name());
  segmentation->setUuid(roVertex->uuid());
  segmentation->restoreState(roState);
  segmentation->setStorage(m_storage);

  auto categoryName = parseCategoryName(roState);

  if (!categoryName.isEmpty())
  {
    auto category = m_analysis->classification()->node(categoryName);

    segmentation->setCategory(category);
  }

  loadExtensions(segmentation);

  m_analysis->add(segmentation);

  return segmentation;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V5::Loader::inflateVertex(DirectedGraph::Vertex roVertex)
{
  DirectedGraph::Vertex vertex = findVertex(m_loadedVertices, roVertex->uuid());

  if (!vertex)
  {
    ReadOnlyVertex *rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());
    switch (rov->type())
    {
      case VertexType::SAMPLE:
      {
        vertex = createSample(roVertex);
        break;
      }
      case VertexType::CHANNEL:
      {
        vertex = createChannel(roVertex);
        break;
      }
      case VertexType::FILTER:
      {
        try
        {
          vertex = createFilter(roVertex);
        } catch (...)
        {
          qDebug() << "Failed to create filter: " << roVertex->uuid() << roVertex->name() << roVertex->state();
        }
        break;
      }
      case VertexType::SEGMENTATION:
      {
        try
        {
          vertex = createSegmentation(roVertex);
        } catch (...)
        {
          qDebug() << "Failed to create segmentation: " << roVertex->name() << roVertex->state();
        }
        break;
      }
      default:
        throw Graph::Unknown_Type_Found();
        break;
    }

    if (vertex != nullptr)
    {
      m_loadedVertices << vertex;
    }
  }

  return vertex;
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadContent()
{
  m_content = std::make_shared<DirectedGraph>();

  QTextStream textStream(readFileFromZip(CONTENT_FILE, m_zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, m_content);

  DirectedGraph::Vertices loadedVertices;

  for (DirectedGraph::Vertex roVertex : m_content->vertices())
  {
    inflateVertex(roVertex);
  }
}
//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadRelations()
{
  auto relations = std::make_shared<DirectedGraph>();

  QTextStream textStream(readFileFromZip(RELATIONS_FILE, m_zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, relations);

  DirectedGraph::Vertices loadedVertices = m_analysis->content()->vertices();

  for (auto edge : relations->edges())
  {
    PersistentSPtr source = findVertex(loadedVertices, edge.source->uuid());
    PersistentSPtr target = findVertex(loadedVertices, edge.target->uuid());

    m_analysis->addRelation(source, target, edge.relationship.c_str());
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::createChannelExtension(ChannelSPtr channel,
                                                const ChannelExtension::Type &type,
                                                const ChannelExtension::InfoCache &cache,
                                                const State &state)
{
  ChannelExtensionSPtr extension;
  try
  {
    extension = m_factory->createChannelExtension(type, cache, state);
    //qDebug() << "Creating Channel Extension" << type;
  } catch (const CoreFactory::Unknown_Type_Exception &e)
  {
    //qDebug() << "Creating ReadOnlyChannelExtension" << type;
    extension = std::make_shared<ReadOnlyChannelExtension>(type, cache, state);
  }
  Q_ASSERT(extension);
  channel->addExtension(extension);
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadExtensions(ChannelSPtr channel)
{
  QString xmlFile = ChannelExtension::ExtensionFilePath(channel.get());

  if (!channel->storage()->exists(xmlFile))
    return;

  QByteArray extensions = channel->storage()->snapshot(xmlFile);

  QXmlStreamReader xml(extensions);

  auto type = QString();
  auto cache = ChannelExtension::InfoCache();
  auto state = State();

  while (!xml.atEnd())
  {
    if (xml.isStartElement())
    {
      if (xml.name() == "Extension")
      {
        type = xml.attributes().value("Type").toString();
        cache = ChannelExtension::InfoCache();
        state = State();
      }
      else
        if (xml.name() == "Info")
        {
          QString name = xml.attributes().value("Name").toString();
          cache[name] = xml.readElementText();
        }
        else
          if (xml.name() == "State")
          {
            state = xml.readElementText();
          }
    }
    else
      if (xml.isEndElement() && xml.name() == "Extension")
      {
        createChannelExtension(channel, type, cache, state);
      }

      xml.readNext();
  }

  if (xml.hasError())
    qDebug() << "channel loadExtensions error:" << xml.errorString();
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::createSegmentationExtension(SegmentationSPtr segmentation,
                                                     const SegmentationExtension::Type &type,
                                                     const SegmentationExtension::InfoCache &cache,
                                                     const State &state)
{
  SegmentationExtensionSPtr extension;
  try
  {
    extension = m_factory->createSegmentationExtension(type, cache, state);
    //qDebug() << "Creating Segmentation Extension" << type;
  } catch (const CoreFactory::Unknown_Type_Exception &e)
  {
    //qDebug() << "Creating ReadOnlySegmentationExtension" << type;
    extension = std::make_shared<ReadOnlySegmentationExtension>(type, cache, state);
  }
  Q_ASSERT(extension);
  segmentation->addExtension(extension);
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadExtensions(SegmentationSPtr segmentation)
{
  QString xmlFile = QString("Extensions/%1.xml").arg(segmentation->uuid());

  if (!segmentation->storage()->exists(xmlFile))
    return;

  QByteArray extensions = segmentation->storage()->snapshot(xmlFile);

  QXmlStreamReader xml(extensions);

  //qDebug() << "Looking for" << segmentation->name() << "extensions:";
  auto type = QString();
  auto cache = SegmentationExtension::InfoCache();
  auto state = State();

  while (!xml.atEnd())
  {
    if (xml.isStartElement())
    {
      if (xml.name() == "Extension")
      {
        type = xml.attributes().value("Type").toString();
        cache = SegmentationExtension::InfoCache();
        state = State();
      }
      else
        if (xml.name() == "Info")
        {
          auto name = xml.attributes().value("Name").toString().replace("_", " ");

          QByteArray base64(xml.readElementText().toStdString().c_str());

          auto data = QByteArray::fromBase64(base64);

          QDataStream in(data);
          in >> cache[name];
        }
        else
          if (xml.name() == "State")
          {
            state = xml.readElementText();
          }
    }
    else
      if (xml.isEndElement() && xml.name() == "Extension")
      {
        createSegmentationExtension(segmentation, type, cache, state);
      }

      xml.readNext();
  }

  if (xml.hasError())
    qDebug() << "segmentation loadExtensions error:" << xml.errorString();
}

//-----------------------------------------------------------------------------
SegFile_V5::SegFile_V5()
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::load(QuaZip&          zip,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler)
{
  Loader loader(zip, factory, handler);

  return loader.load();
}

//-----------------------------------------------------------------------------
void SegFile_V5::save(AnalysisPtr analysis, QuaZip& zip, ErrorHandlerSPtr handler)
{
  try
  {
    addFileToZip(FORMAT_INFO_FILE, formatInfo(), zip, handler);
  }
  catch (const IO_Error_Exception &e)
  {
    if (handler)
      handler->error("Error while saving Analysis Format Information.");

    throw (e);
  }

  QByteArray classification;
  try
  {
    classification = ClassificationXML::dump(analysis->classification(), handler);
  }
  catch (const IO_Error_Exception &e)
  {
    if (handler)
      handler->error("Error while saving Analysis Classification.");

    throw(e);
  }

  try
  {
    addFileToZip(CLASSIFICATION_FILE, classification, zip, handler);
  }
  catch (const IO_Error_Exception &e)
  {
    if (handler)
      handler->error("Error while saving Analysis Classification.");

    throw(e);
  }

  std::ostringstream content;
  write(analysis->content(), content);
  try
  {
    addFileToZip(CONTENT_FILE, content.str().c_str(), zip, handler);
  }
  catch (const IO_Error_Exception &e)
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline.");

    throw (e);
  }

  std::ostringstream relations;
  write(analysis->relationships(), relations);
  try
  {
    addFileToZip(RELATIONS_FILE, relations.str().c_str(), zip, handler);
  }
  catch (const IO_Error_Exception &e)
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline.");

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
      }
      catch (const IO_Error_Exception &e)
      {
        throw (e);
      }
    }
  }

  if(analysis->storage() != nullptr)
    for (auto data : analysis->storage()->snapshots(QString("Extra"), TemporalStorage::Mode::Recursive))
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      }
      catch (const IO_Error_Exception &e)
      {
        if (handler)
          handler->warning("Error while saving Extra content.");

        throw (e);
      }
    }
}

