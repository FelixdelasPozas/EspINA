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
#include "ProgressReporter.h"
#include <EspinaConfig.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Connections.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Extensions/ReadOnlySegmentationExtension.h>
#include <Core/Analysis/Extensions/ReadOnlyStackExtension.h>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Analysis/Filters/ReadOnlyFilter.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/QStringUtils.h>

// Qt
#include <QDataStream>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;
using namespace ESPINA::IO::Graph;

const QString SegFile::SegFile_V5::FORMAT_INFO_FILE = "formatInfo.ini";

const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";
const QString CLASSIFICATION_FILE = "classification.xml";
const QString CONNECTIONS_FILE    = ConnectionStorage::connectionsFileName();
const QString CURRENT_SEG_FILE_VERSION = "6";

const int FIX_SOURCE_INPUTS_SEG_FILE_VERSION = 5;

const unsigned CLASSIFICATION_PROGRESS =  5;
const unsigned SNAPSHOT_PROGRESS       = 20;
const unsigned CONTENT_PROGRESS        = 70;
const unsigned RELATIONS_PROGRESS      = 99;

const float SNAPSHOT_PROGRESS_CHUNK  = SNAPSHOT_PROGRESS  - CLASSIFICATION_PROGRESS;
const float CONTENT_PROGRESS_CHUNK   = CONTENT_PROGRESS   - SNAPSHOT_PROGRESS;
const float RELATIONS_PROGRESS_CHUNK = RELATIONS_PROGRESS - CONTENT_PROGRESS;

const QString SEG_FILE_VERSION = "SegFile Version";

//-----------------------------------------------------------------------------
QByteArray formatInfo()
{
  QByteArray info;

  QTextStream infoStream(&info);

  infoStream << QString("%1=%2").arg(SEG_FILE_VERSION).arg(CURRENT_SEG_FILE_VERSION) << endl;
  infoStream << QString("ESPINA Version=%1").arg(ESPINA_VERSION) << endl;

  return info;
}

//-----------------------------------------------------------------------------
int segFileVersion(const QString &info)
{
  const int EQUAL_LENGTH = 1;
  auto start = info.indexOf(SEG_FILE_VERSION)
             + SEG_FILE_VERSION.length()
             + EQUAL_LENGTH;
  auto n     = info.indexOf("\n", start) - start;

  return info.mid(start, n).toInt();
}

//-----------------------------------------------------------------------------
SegFile_V5::Loader::Loader(QuaZip           &zip,
                           CoreFactorySPtr  factory,
                           ProgressReporter *reporter,
                           ErrorHandlerSPtr  handler,
                           const LoadOptions options)
: m_zip            (zip)
, m_factory        {factory}
, m_reporter       {reporter}
, m_handler        {handler}
, m_options        {options}
, m_analysis       {new Analysis()}
, m_dataFactory    {new RawDataFactory()}
, m_fixSourceInputs{false}
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::Loader::load()
{
  reportProgress(0);

  m_storage = m_factory->createTemporalStorage();

  if (!m_zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (m_handler)
    {
      m_handler->error(QObject::tr("Could not load analysis classification"));
    }

    auto what    = QObject::tr("Classification not found.");
    auto details = QObject::tr("SegFile_V5::load() -> Can't load classification.");
    throw EspinaException(what, details);
  }

  try
  {
    auto currentFile    = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
    auto classification = ClassificationXML::parse(currentFile, m_handler);
    m_analysis->setClassification(classification);
  }
  catch (const EspinaException &e)
  {
    if (m_handler)
    {
      m_handler->error(QObject::tr("Error while loading classification"));
    }

    throw(e);
  }

  reportProgress(CLASSIFICATION_PROGRESS);

  unsigned long i = 0;
  const unsigned long total = m_zip.getFileNameList().size();

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    QString file = m_zip.getCurrentFileName();

    if (file == FORMAT_INFO_FILE)
    {
      auto info = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
      if (segFileVersion(info) <= FIX_SOURCE_INPUTS_SEG_FILE_VERSION)
      {
        m_fixSourceInputs = true;
      }
    }
    else
    {
      if (file != CLASSIFICATION_FILE && file != CONTENT_FILE && file != RELATIONS_FILE)
      {
        auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);

        // FIX: Windows doesn't allow some characters in filenames that Linux does and were used in previous versions.
        file = file.replace(">", "_"); // TabularReport save key information file.

        m_storage->saveSnapshot(SnapshotData(file, currentFile));
      }
    }

    hasFile = m_zip.goToNextFile();

    reportProgress(CLASSIFICATION_PROGRESS + (SNAPSHOT_PROGRESS_CHUNK*(++i)/total));
  }

  loadContent();

  loadRelations();

  m_analysis->setStorage(m_storage);

  loadConnections();

  reportProgress(100);

  return m_analysis;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V5::Loader::findVertex(DirectedGraph::Vertices vertices, Persistent::Uuid uuid)
{
  auto it = std::find_if(vertices.constBegin(), vertices.constEnd(), [&uuid](const DirectedGraph::Vertex &vertex) {return vertex->uuid() == uuid; });
  if(it != vertices.constEnd()) return *it;

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
  DirectedGraph::Edges inputConnections = m_content->inEdges(roVertex);

  InputSList inputs;
  int lastOutput = -1; // Debug
  for (auto edge : inputConnections)
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

    if (inputNumber <= lastOutput)
    {
      qWarning() << "Unordered outputs";
    }
    lastOutput = inputNumber;

    inputs << getInput(filter, id);
  }

  FilterSPtr filter;
  try
  {
    filter = m_factory->createFilter(inputs, roVertex->name());

    // NOTE: These modify the original 'inputs' value adding a default one, but only if 'inputs' is empty.
    //       Fixes SourceFilter inputs and ReadOnlyFilters (belonging to SegmhaImporter). Could be done in
    //       another way but it's better not to reference or hack directly values (like using the TYPE of
    //       SegmhaImporter plugin, not known to Core library).
    if(m_fixSourceInputs && inputs.isEmpty() && m_sourceInput != nullptr)
    {
      inputs << m_sourceInput->asInput();

      auto sourceFilter = dynamic_cast<SourceFilter *>(filter.get());
      if (sourceFilter)
      {
        sourceFilter->setInputs(inputs);
      }

      auto readOnlyFilter = dynamic_cast<ReadOnlyFilter *>(filter.get());
      if(readOnlyFilter)
      {
        readOnlyFilter->setInputs(inputs);
      }
    }
  }
  catch (const EspinaException &e)
  {
    filter = std::make_shared<ReadOnlyFilter>(inputs, roVertex->name());
    filter->setDataFactory(m_dataFactory);
  }

  filter->setErrorHandler(m_handler);
  filter->setName(roVertex->name());
  filter->setUuid(roVertex->uuid());
  filter->restoreState(roVertex->state());

  auto reader = std::dynamic_pointer_cast<VolumetricStreamReader>(filter);
  if(reader)
  {
    if(m_options.contains(VolumetricStreamReader::STREAMING_OPTION))
    {
      reader->setStreaming(m_options.value(VolumetricStreamReader::STREAMING_OPTION).toBool() == true);
    }
  }
  filter->setStorage(m_storage);
  filter->restorePreviousOutputs();

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V5::Loader::findOutput(DirectedGraph::Vertex roVertex)
{
  QPair<FilterSPtr, Output::Id> output;

  auto inputConections = m_content->inEdges(roVertex);
  Q_ASSERT(inputConections.size() == 1);

  auto edge = inputConections.first();

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
  auto channel  = m_factory->createChannel(filter, outputId);

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
  const auto params = state.split(";");
  const auto name = params[2].split("=")[1];

  return Core::Utils::simplifyString(name);
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V5::Loader::createSegmentation(DirectedGraph::Vertex roVertex)
{
  auto roOutput = findOutput(roVertex);

  auto filter   = roOutput.first;
  auto outputId = roOutput.second;

  if (!filter)
  {
    auto what    = QObject::tr("Invalid input, filter is null in vertex %1").arg(roVertex->name());
    auto details = QObject::tr("SegFile_V5::createSegmentation() -> Invalid input from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());
    throw EspinaException(what, details);
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
  else
  {
    auto what    = QObject::tr("Invalid input, category is null in vertex %1").arg(roVertex->name());
    auto details = QObject::tr("SegFile_V5::createSegmentation() -> Invalid input from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());
    throw EspinaException(what, details);
  }

  loadExtensions(segmentation);

  m_analysis->add(segmentation);

  return segmentation;
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V5::Loader::inflateVertex(DirectedGraph::Vertex roVertex)
{
  auto vertex = findVertex(m_loadedVertices, roVertex->uuid());

  if (!vertex)
  {
    auto rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());
    switch (rov->type())
    {
      case VertexType::SAMPLE:
      {
        vertex = createSample(roVertex);
        break;
      }
      case VertexType::CHANNEL:
      {
        try
        {
          vertex = m_sourceInput = createChannel(roVertex);
        }
        catch (const EspinaException &e)
        {
          auto what    = QObject::tr("Failed to create channel: %1. ").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V5::inflateVertex() -> Can't create channel from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw EspinaException(what, details);
        }
        break;
      }
      case VertexType::FILTER:
      {
        try
        {
          vertex = createFilter(roVertex);
        }
        catch (const EspinaException &e)
        {
          auto what    = QObject::tr("Failed to create filter: %1. ").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V5::inflateVertex() -> Can't create filter from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw EspinaException(what, details);
        }
        break;
      }
      case VertexType::SEGMENTATION:
      {
        try
        {
          vertex = createSegmentation(roVertex);
        }
        catch (const EspinaException &e)
        {
          auto what    = QObject::tr("Failed to create segmentation: %1. ").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V5::inflateVertex() -> Can't create segmentation from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw EspinaException(what, details);
        }
        break;
      }
      default:
        auto what    = QObject::tr("Unknown vertex type: %1. ").arg(static_cast<int>(rov->type()));
        auto details = QObject::tr("SegFile_V5::inflateVertex() -> Unknown type from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());

        throw EspinaException(what, details);
        break;
    }

    if (vertex == nullptr)
    {
       auto what    = QObject::tr("Unable to inflate vertex, type: %1. ").arg(static_cast<int>(rov->type()));
       auto details = QObject::tr("SegFile_V5::inflateVertex() -> Unable to inflate vertex, type %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid().toString()).arg(roVertex->state());

       throw EspinaException(what, details);
    }
	
	m_loadedVertices << vertex;
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

  auto vertices = m_content->vertices();
  const unsigned long total = vertices.size();

  auto sorter = [](const DirectedGraph::Vertex &lv, const DirectedGraph::Vertex &rv)
  {
    auto dlv = dynamic_cast<IO::Graph::ReadOnlyVertex *>(lv.get());
    auto drv = dynamic_cast<IO::Graph::ReadOnlyVertex *>(rv.get());

    if(!dlv || !drv) return false;

    if(dlv->type() == IO::Graph::VertexType::SAMPLE) return true;
    if(drv->type() == IO::Graph::VertexType::SAMPLE) return false;

    if(dlv->type() == IO::Graph::VertexType::CHANNEL)
    {
      return drv->type() != IO::Graph::VertexType::CHANNEL;
    }

    return drv->type() == IO::Graph::VertexType::CHANNEL;
  };

  // NOTE: loads channel elements first.
  std::sort(vertices.begin(), vertices.end(), sorter);

  unsigned long i = 0;
  for (DirectedGraph::Vertex roVertex : vertices)
  {
    inflateVertex(roVertex);

    reportProgress(SNAPSHOT_PROGRESS + (CONTENT_PROGRESS_CHUNK*(++i)/total));
  }
}
//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadRelations()
{
  auto relations = std::make_shared<DirectedGraph>();

  QTextStream textStream(readFileFromZip(RELATIONS_FILE, m_zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, relations);

  auto loadedVertices = m_analysis->content()->vertices();

  unsigned long i = 0;
  const unsigned long total = relations->edges().size();
  for (auto edge : relations->edges())
  {
    PersistentSPtr source = findVertex(loadedVertices, edge.source->uuid());
    PersistentSPtr target = findVertex(loadedVertices, edge.target->uuid());

    m_analysis->addRelation(source, target, edge.relationship.c_str());

    reportProgress(CONTENT_PROGRESS + (RELATIONS_PROGRESS_CHUNK*(++i)/total));
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::createStackExtension(ChannelSPtr channel,
                                              const StackExtension::Type &type,
                                              const StackExtension::InfoCache &cache,
                                              const State &state)
{
  StackExtensionSPtr extension = nullptr;

  try
  {
    extension = m_factory->createStackExtension(type, cache, state);
  }
  catch (const EspinaException &e)
  {
    extension = std::make_shared<ReadOnlyStackExtension>(type, cache, state);
  }
  Q_ASSERT(extension);

  auto extensions = channel->extensions();
  extensions->add(extension);
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadExtensions(ChannelSPtr channel)
{
  auto xmlFile = StackExtension::ExtensionFilePath(channel.get());

  if (!channel->storage()->exists(xmlFile)) return;

  auto extensions = channel->storage()->snapshot(xmlFile);

  // in some SEG files the extension xml files were corrupted, need to clean them or the extensions won't be created.
  const auto token = QByteArray("</Channel>") + '\n';
  if(!extensions.endsWith(token))
  {
    auto index = extensions.indexOf(token, 0);
    Q_ASSERT(index != -1);
    extensions.remove(index + token.length(), extensions.size() - index - token.length());
  }

  QXmlStreamReader xml(extensions);

  auto type  = QString();
  auto cache = StackExtension::InfoCache();
  auto state = State();

  while (!xml.atEnd())
  {
    if (xml.isStartElement())
    {
      if (xml.name() == "Extension")
      {
        type = xml.attributes().value("Type").toString();
        cache = StackExtension::InfoCache();
        state = State();
      }
      else
      {
        if (xml.name() == "Info")
        {
          QString name = xml.attributes().value("Name").toString();
          cache[name] = xml.readElementText();
        }
        else
        {
          if (xml.name() == "State")
          {
            state = xml.readElementText();
          }
        }
      }
    }
    else
    {
      if (xml.isEndElement() && xml.name() == "Extension")
      {
        createStackExtension(channel, type, cache, state);
      }
    }

    xml.readNext();
  }

  if (xml.hasError())
  {
    qWarning() << "SEGFILE_V5 - Channel load extensions xml error:" << xml.errorString();
  }
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
  }
  catch (const EspinaException &e)
  {
    extension = std::make_shared<ReadOnlySegmentationExtension>(type, cache, state);
  }
  Q_ASSERT(extension);
  segmentation->extensions()->add(extension);
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadExtensions(SegmentationSPtr segmentation)
{
  auto xmlFile = QString("Extensions/%1.xml").arg(segmentation->uuid().toString());

  if (!segmentation->storage()->exists(xmlFile)) return;

  auto extensions = segmentation->storage()->snapshot(xmlFile);

  // in some SEG files the extension xml files were corrupted, need to clean them or the extensions won't be created.
  const auto token = QByteArray("</Segmentation>") + '\n';
  if(!extensions.endsWith(token))
  {
    auto index = extensions.indexOf(token, 0);
    Q_ASSERT(index != -1);
    extensions.remove(index + token.length(), extensions.size() - index - token.length());
  }

  QXmlStreamReader xml(extensions);

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

        fixVersion2_1_8(type);

        cache = SegmentationExtension::InfoCache();
        state = State();
      }
      else
      {
        if (xml.name() == "Info")
        {
          auto name = xml.attributes().value("Name").toString().replace("_", " ");

          QByteArray base64(xml.readElementText().toStdString().c_str());

          auto data = QByteArray::fromBase64(base64);

          QDataStream in(data);
          in >> cache[name];
        }
        else
        {
          if (xml.name() == "State")
          {
            state = xml.readElementText();
          }
        }
      }
    }
    else
    {
      if (xml.isEndElement() && xml.name() == "Extension")
      {
        createSegmentationExtension(segmentation, type, cache, state);
      }
    }

    xml.readNext();
  }

  if (xml.hasError())
  {
    qWarning() << "SEGFILE_V5 - Segmentation load extensions xml error:" << xml.errorString();
  }
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::loadConnections()
{
  m_analysis->loadConnections();
}

//-----------------------------------------------------------------------------
void SegFile_V5::Loader::reportProgress(unsigned int currentProgress)
{
  static unsigned int progress = std::numeric_limits<unsigned int>::max();
  if (m_reporter && (progress != currentProgress))
 {
	progress = currentProgress;
	m_reporter->setProgress(progress);
  }
}

//-----------------------------------------------------------------------------
SegFile_V5::SegFile_V5()
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V5::load(QuaZip&           zip,
                              CoreFactorySPtr   factory,
                              ProgressReporter *reporter,
                              ErrorHandlerSPtr  handler,
                              const LoadOptions options)
{
  Loader loader(zip, factory, reporter, handler, options);

  return loader.load();
}

//-----------------------------------------------------------------------------
void SegFile_V5::save(AnalysisPtr analysis,
                      QuaZip& zip,
                      ProgressReporter *reporter,
                      ErrorHandlerSPtr handler)
{
  if (reporter)
  {
    reporter->setProgress(0);
  }

  try
  {
    addFileToZip(FORMAT_INFO_FILE, formatInfo(), zip, handler);
  }
  catch (const EspinaException &e)
  {
    if (handler)
    {
      handler->error("Error while saving Analysis Format Information.");
    }

    throw (e);
  }

  QByteArray classification;
  try
  {
    classification = ClassificationXML::dump(analysis->classification(), handler);
  }
  catch (const EspinaException &e)
  {
    if (handler)
    {
      handler->error("Error while dumping Analysis classification to byte array.");
    }

    throw(e);
  }

  try
  {
    addFileToZip(CLASSIFICATION_FILE, classification, zip, handler);
  }
  catch (const EspinaException &e)
  {
    if (handler)
    {
      handler->error("Error while saving Analysis classification.");
    }

    throw(e);
  }

  if (reporter)
  {
    reporter->setProgress(CLASSIFICATION_PROGRESS);
  }

  auto storage = analysis->storage();

  if(analysis->saveConnections())
  {
    try
    {
      addFileToZip(CONNECTIONS_FILE, storage->snapshot(CONNECTIONS_FILE), zip, handler);
    }
    catch(const EspinaException &e)
    {
      if (handler)
      {
        handler->error("Error while saving connections data.");
      }

      throw (e);
    }
  }

  std::ostringstream content;
  write(analysis->content(), content);
  try
  {
    addFileToZip(CONTENT_FILE, content.str().c_str(), zip, handler);
  }
  catch (const EspinaException &e)
  {
    if (handler)
    {
      handler->error("Error while saving analysis content graph.");
    }

    throw (e);
  }

  if (reporter)
  {
    reporter->setProgress(40);
  }

  std::ostringstream relations;
  write(analysis->relationships(), relations);
  try
  {
    addFileToZip(RELATIONS_FILE, relations.str().c_str(), zip, handler);
  }
  catch (const EspinaException &e)
  {
    if (handler)
    {
      handler->error("Error while saving analysis relationships graph.");
    }

    throw (e);
  }

  if (reporter)
  {
    reporter->setProgress(60);
  }

  int i = 0;
  int total = analysis->content()->vertices().size();

  for(auto v : analysis->content()->vertices())
  {
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.get());

    try
    {
      for(auto data : item->snapshot())
      {
        addFileToZip(data.first, data.second, zip, handler);
      }
    }
    catch (const EspinaException &e)
    {
      if(handler)
      {
        handler->error(QObject::tr("Unable to save data to seg file."));
      }

      throw(e);
    }

    if (reporter)
    {
      reporter->setProgress(60 + 20*(++i)/total);
    }
  }

  if(storage != nullptr)
  {

    Snapshot files;
    files << storage->snapshots(QString("Extra"), TemporalStorage::Mode::RECURSIVE);
    files << storage->snapshots(QString("Settings"), TemporalStorage::Mode::RECURSIVE);

    i = 0;
    total = files.size();

    for (auto data : files)
    {
      try
      {
        addFileToZip(data.first, data.second, zip, handler);
      }
      catch (const EspinaException &e)
      {
        if (handler)
        {
          handler->warning(QString("Error while saving storage additional contents: %1").arg(data.first));
        }

        throw (e);
      }

      if (reporter)
      {
        reporter->setProgress(80 + 20*(++i)/total);
      }
    }
  }

  if (reporter)
  {
    reporter->setProgress(100);
  }
}

//--------------------------------------------------------------------
void SegFile_V5::Loader::fixVersion2_1_8(Core::SegmentationExtension::Type& type)
{
  // NOTE: hard-coded signatures to avoid having a dependency with SAS plugin. Damn... I hate these things.
  if(type == "AppositionSurfaceExtensionInformation")
  {
    type = "AppositionSurface";
  }
}
