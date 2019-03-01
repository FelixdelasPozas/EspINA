/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "SegFile_V4.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Data/Volumetric/StreamedVolume.hxx>
#include <Core/Factory/CoreFactory.h>
#include <Core/IO/SegFile.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Core/IO/ClassificationXML.h>
#include <Core/Analysis/Filters/ReadOnlyFilter.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Readers/ChannelReader.h>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include "ProgressReporter.h"

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;
using namespace ESPINA::IO::Graph;

const QString SegFile::SegFile_V4::FORMAT_INFO_FILE = "settings.ini";

const QString TRACE_FILE          = "trace.dot";
const QString CLASSIFICATION_FILE = "taxonomy.xml";
const QString FILE_VERSION        = "version"; //backward compatibility

const unsigned CLASSIFICATION_PROGRESS =  5;
const unsigned TRACE_PROGRESS          = 40;
const unsigned SNAPSHOT_PROGRESS       = 70;
const unsigned RELATIONS_PROGRESS      = 99;

const float TRACE_PROGRESS_CHUNK     = TRACE_PROGRESS     - CLASSIFICATION_PROGRESS;
const float SNAPSHOT_PROGRESS_CHUNK  = SNAPSHOT_PROGRESS  - TRACE_PROGRESS;
const float RELATIONS_PROGRESS_CHUNK = RELATIONS_PROGRESS - SNAPSHOT_PROGRESS;

//-----------------------------------------------------------------------------
SegFile_V4::Loader::Loader(QuaZip& zip,
                           CoreFactorySPtr   factory,
                           ProgressReporter *reporter,
                           ErrorHandlerSPtr  handler,
                           const LoadOptions options)
: m_zip        (zip)
, m_factory    {factory}
, m_reporter   {reporter}
, m_handler    {handler}
, m_options    {options}
, m_dataFactory{new MarchingCubesFromFetchedVolumetricData()}
, m_analysis   {new Analysis()}
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V4::Loader::load()
{
  reportProgress(0);

  m_storage = m_factory->createTemporalStorage();
  m_analysis->setStorage(m_storage);

  m_vertexUuids.clear();
  m_trace.reset();
  m_loadedVertices.clear();

  if (!m_zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (m_handler)
    {
      m_handler->error(QObject::tr("Could not load analysis classification"));
    }

    auto what    = QObject::tr("Classification not found.");
    auto details = QObject::tr("SegFile_V4::load() -> Can't load classification.");
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

    throw (e);
  }

  reportProgress(CLASSIFICATION_PROGRESS);

  loadTrace();

  int i = 0;
  int total = m_zip.getFileNameList().size();

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    auto file = m_zip.getCurrentFileName();

    if ( file != FORMAT_INFO_FILE
      && file != CLASSIFICATION_FILE
      && file != TRACE_FILE
      && !file.contains("Outputs/")
      && !file.contains("SegmentationVolume/")
      && !file.contains("MeshOutputType/"))
    {
      auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);

      // FIX: Windows doesn't allow some characters in filenames that Linux does and were used in previous versions.
      file = file.replace(">", "_"); // TabularReport save key information file.

      m_storage->saveSnapshot(SnapshotData(file, currentFile));
    }

    reportProgress(TRACE_PROGRESS + SNAPSHOT_PROGRESS_CHUNK*(++i)/total);

    hasFile = m_zip.goToNextFile();
  }

  restoreRelations();

  return m_analysis;
}

//-----------------------------------------------------------------------------
struct Vertex_Not_Found_Exception{};

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V4::Loader::findInflatedVertexByIdV4(int id) const
{
  for(auto vertex : m_loadedVertices)
  {
    if (vertex->uuid() == m_vertexUuids[id]) return vertex;
  }

  return DirectedGraph::Vertex();
}

//-----------------------------------------------------------------------------
SampleSPtr SegFile_V4::Loader::createSample(DirectedGraph::Vertex roVertex)
{
  auto sample = m_factory->createSample();

  sample->setName(roVertex->name());
  sample->restoreState(roVertex->state());
  sample->setStorage(m_storage);

  m_analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V4::Loader::createFilter(DirectedGraph::Vertex roVertex)
{
  auto inputConections = m_trace->inEdges(roVertex);

  InputSList inputs;
  for(auto edge : inputConections)
  {
    auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
    auto input     = inflateVertexV4(vertex_v4);

    auto inputFilter = std::dynamic_pointer_cast<Filter>(input);
    if (inputFilter)
    {
      Output::Id id = atoi(edge.relationship.c_str());

      // Here it is safe to create the outputs because they already existed
      // In addition, it may be the case an update couldn't be executed if
      // traceability was disabled
      if (!inputFilter->m_outputs.contains(id))
      {
        inputFilter->m_outputs[id] = std::make_shared<Output>(inputFilter.get(), id, NmVector3());
      }

      inputs << getInput(inputFilter, id);
    }
  }

  FilterSPtr filter;
  try
  {
    filter = m_factory->createFilter(inputs, roVertex->name());
  }
  catch (...)
  {
    filter = std::make_shared<ReadOnlyFilter>(inputs, roVertex->name());
    filter->setDataFactory(m_dataFactory);
  }
  filter->setErrorHandler(m_handler);
  filter->setName(roVertex->name());
  filter->restoreState(roVertex->state());
  filter->setStorage(m_storage);

  auto state  = roVertex->state();
  for(auto arg : state.split(";"))
  {
    auto parts = arg.split("=");
    if ("ID" == parts[0])
    {
      createFilterOutputsFile(filter, parts[1].toInt());
    }
  }
  filter->restorePreviousOutputs();

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V4::Loader::findOutput(DirectedGraph::Vertex   roVertex,
                                                             const QString          &linkName)
{
  QPair<FilterSPtr, Output::Id> output;

  auto inputConections = m_trace->inEdges(roVertex, linkName);
  Q_ASSERT(inputConections.size() == 1);

  auto edge = inputConections.first();

  auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
  auto input     = inflateVertexV4(vertex_v4);

  output.first  = std::dynamic_pointer_cast<Filter>(input);
  output.second = parseOutputId(roVertex->state());

  return output;
}

//-----------------------------------------------------------------------------
ChannelSPtr SegFile_V4::Loader::createChannel(DirectedGraph::Vertex roVertex)
{
  auto inputConections = m_trace->inEdges(roVertex, "Volume");
  Q_ASSERT(inputConections.size() == 1);

  auto edge = inputConections.first();

  auto vertex_v4 = edge.source;
  auto filter    = std::dynamic_pointer_cast<Filter>(inflateVertexV4(vertex_v4));
  auto reader    = std::dynamic_pointer_cast<VolumetricStreamReader>(filter);
  if(reader)
  {
    reader->setStreaming(m_options.contains(VolumetricStreamReader::STREAMING_OPTION) &&
                         m_options.value(VolumetricStreamReader::STREAMING_OPTION).toBool() == true);
  }
  filter->update(); // Existing outputs weren't stored in previous versions

  auto channel = m_factory->createChannel(filter, 0);

  channel->setName(roVertex->name());
  channel->restoreState(roVertex->state() + vertex_v4->state());
  channel->setStorage(m_storage);

  m_analysis->add(channel);

  return channel;
}

//-----------------------------------------------------------------------------
QString SegFile_V4::Loader::parseCategoryName(const State& state)
{
  QString category;

  auto params = state.split(";");

  for (auto param : params)
  {
    auto tokens = param.split("=");
    if ("Taxonomy" == tokens[0])
    {
      category = tokens[1];
    }
  }

  return category;
}

//-----------------------------------------------------------------------------
int SegFile_V4::Loader::parseOutputId(const State& state)
{
  int id = 0;

  auto params = state.split(";");

  for (auto param : params)
  {
    auto tokens = param.split("=");
    if ("Output" == tokens[0])
    {
      id = tokens[1].toInt();
    }
  }

  return id;
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V4::Loader::createSegmentation(DirectedGraph::Vertex roVertex)
{
  auto roOutput = findOutput(roVertex, "CreateSegmentation");

  auto filter       = roOutput.first;
  auto outputId     = roOutput.second;
  auto segmentation = m_factory->createSegmentation(filter, outputId);

  auto roState = roVertex->state();
  segmentation->setName(roVertex->name());
  segmentation->restoreState(roState);
  segmentation->setStorage(m_storage);

  auto categoryName = parseCategoryName(roState);

  if (!categoryName.isEmpty())
  {
    auto category = m_analysis->classification()->node(categoryName);

    segmentation->setCategory(category);
  }

  m_analysis->add(segmentation);

  return segmentation;
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::loadTrace()
{
  m_trace = std::make_shared<DirectedGraph>();

  QTextStream textStream(readFileFromZip(TRACE_FILE, m_zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, m_trace);

  int i = 0;
  int total = m_trace->vertices().size();

  for(DirectedGraph::Vertex roVertex : m_trace->vertices())
  {
    inflateVertexV4(roVertex);

    reportProgress(SNAPSHOT_PROGRESS + TRACE_PROGRESS_CHUNK*(++i)/total);
  }
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V4::Loader::inflateVertexV4(DirectedGraph::Vertex roVertex)
{
  auto rov    = dynamic_cast<ReadOnlyVertex *>(roVertex.get());
  auto vertex = findInflatedVertexByIdV4(rov->vertexId());

  if (!vertex)
  {
    switch(rov->type())
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
          vertex = createChannel(roVertex);
        }
        catch(const EspinaException & e)
        {
          auto what    = QObject::tr("Unable to create channel, vertex: %1").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V4::inflateVertexV4() -> Unable to create channel from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw(EspinaException(what, details));
        }
        break;
      }
      case VertexType::FILTER:
      {
        try
        {
          vertex = createFilter(roVertex);
        }
        catch(const EspinaException &e)
        {
          auto what    = QObject::tr("Unable to create filter, vertex: %1").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V4::inflateVertexV4() -> Unable to create filter from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw(EspinaException(what, details));
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
          auto what    = QObject::tr("Unable to create segmentation, vertex: %1").arg(roVertex->name());
          auto details = QObject::tr("SegFile_V4::inflateVertexV4() -> Unable to create segmentation from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid()).arg(roVertex->state());

          what += QString(e.what());
          details += e.details();

          throw(EspinaException(what, details));
        }
        break;
      }
      default:
        auto what    = QObject::tr("Unknown vertex type: %1").arg(static_cast<int>(rov->type()));
        auto details = QObject::tr("SegFile_V4::inflateVertexV4() -> Unknown type from vertex %1, uuid: %2, state %3").arg(roVertex->name()).arg(roVertex->uuid()).arg(roVertex->state());

        throw EspinaException(what, details);
        break;
    }

    if (vertex != nullptr)
    {
      auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(roVertex);
      m_loadedVertices << vertex;
      m_vertexUuids[vertex_v4->vertexId()] = vertex->uuid();
    }
  }

  return vertex;
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::createSegmentations()
{
  for (auto roVertex : m_pendingSegmentationVertices)
  {
    auto vertex = createSegmentation(roVertex);

    Q_ASSERT(vertex != nullptr);

    auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(roVertex);
    m_loadedVertices << vertex;
    m_vertexUuids[vertex_v4->vertexId()] = vertex->uuid();
  }
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::restoreRelations()
{
  int i = 0;
  int total = m_trace->edges().size();

  for(auto edge : m_trace->edges())
  {
    auto source_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
    PersistentSPtr source = findInflatedVertexByIdV4(source_v4->vertexId());

    auto target_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.target);
    PersistentSPtr target = findInflatedVertexByIdV4(target_v4->vertexId());

    if (!isFilter(source_v4) && !isFilter(target_v4))
    {
      std::string relation = edge.relationship;
      try
      {
        if (isSample(source_v4) && isSegmentation(target_v4) && relation == "where")
        {
          relation = Sample::CONTAINS.toStdString();
        }
        m_analysis->addRelation(source, target, relation.c_str());
      }
      catch (...)
      {
        qWarning() << "Invalid Relationship: " << relation.c_str();
      }
    }

    reportProgress(SNAPSHOT_PROGRESS + RELATIONS_PROGRESS_CHUNK*(++i)/total);
  }
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::createFilterOutputsFile(FilterSPtr filter, int filterVertex)
{
  QString outputsFile;
  QMap<int, QList<QByteArray>> trcFiles;

  const QUuid uuid = filter->uuid();

  QMap<int, Bounds>    bounds;
  QMap<int, NmVector3> spacings;
  QMap<int, QList<QPair<int, Bounds>>> volumeEditedRegions;

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    auto file = m_zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE
      && file != CLASSIFICATION_FILE
      && file != TRACE_FILE
      && !file.endsWith("/"))
    {
      // Translate filenames to expected format
      if (file.contains("Outputs/"))
      {
        auto trcFile = file.remove(0, 8);
        auto tokens  = trcFile.split("_");
        auto vertex  = tokens[0].toInt();
        if (vertex == filterVertex)
        {
          auto output = tokens[1].split(".")[0].toInt();
          outputsFile = QString("Filters/%1/outputs.xml").arg(uuid.toString());

          auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
          trcFiles[output].append(currentFile);
        }
      }
      else
      {
        if (file.contains("SegmentationVolume/"))
        {
          auto oldFile        = file.remove(0, 19);
          auto parts          = oldFile.split(".");
          auto ids            = parts[0].split("_");
          auto extension      = parts[1].toLower();
          auto vertex         = ids[0].toInt();
          auto output         = ids[1].toInt();
          auto editedRegionId = ids.size() == 3? ids[2].toInt():-1;

          if (vertex == filterVertex)
          {
            auto newFile = QString("Filters/%1/").arg(uuid.toString());
            if (extension == "mhd")
            {
              if(editedRegionId >= 0)
              {
                newFile += QString("%1_VolumetricData_EditedRegion_%2.mhd").arg(output).arg(editedRegionId);
              }
              else
              {
                newFile += QString("%1_VolumetricData.mhd").arg(output);
              }
            }
            else if (extension == "raw")
            {
              newFile += oldFile; // mhd internal references to raw files are not modified
            }

            auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
            m_storage->saveSnapshot(SnapshotData(newFile, currentFile));

            if (extension == "mhd")
            {
              StreamedVolume<itkVolumeType> volume(m_storage->absoluteFilePath(newFile));
              if (editedRegionId >= 0)
              {
                volumeEditedRegions[output] << QPair<int, Bounds>(editedRegionId, volume.bounds());
              }
              else
              {
                bounds.insert(output, volume.bounds());
                spacings.insert(output, volume.bounds().spacing());
              }
            }
          }
        }
        else
        {
          if (file.contains("MeshOutputType/"))
          {
            auto oldFile = file.remove(0, 15);
            auto parts   = oldFile.split("_");
            auto vertex  = parts[0].toInt();
            if (vertex == filterVertex)
            {
              auto newFile = QString("Filters/%1/").arg(uuid.toString());
              Q_ASSERT(parts[1].endsWith(".vtp", Qt::CaseInsensitive));
              auto strings = parts[1].split("-");
              newFile += QString("MeshData_%1.vtp").arg(strings[0]);

              auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
              m_storage->saveSnapshot(SnapshotData(newFile, currentFile));
            }
          }
        }
      }
    }

    hasFile = m_zip.goToNextFile();
  }

  if (!trcFiles.isEmpty())
  {
    QByteArray buffer;
    QXmlStreamWriter xml(&buffer);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("Filter");
    xml.writeAttribute("name", filter->name());
    for(auto output : trcFiles.keys())
    {
      for (auto trc : trcFiles[output])
      {
        xml.writeStartElement("Output");
        xml.writeAttribute("id",      QString::number(output));
        xml.writeAttribute("bounds",  bounds[output].toString());
        xml.writeAttribute("spacing", spacings[output].toString());

        QString content(trc);

        QStringList lines = content.split(QRegExp("[\r\n]"));
        for(int i = 0; i < lines.size(); ++i)
        {
          if ("SegmentationVolume" == lines[i])
          {
            xml.writeStartElement("Data");
            xml.writeAttribute("type",  "VolumetricData");
            xml.writeAttribute("bounds", bounds[output].toString());
            for(auto editedRegion : volumeEditedRegions[output])
            {
              auto editedBounds = editedRegion.second;
              if (editedBounds.areValid())
              {
                xml.writeStartElement("EditedRegion");
                xml.writeAttribute("id",     QString::number(editedRegion.first));
                xml.writeAttribute("bounds", editedBounds.toString());
                xml.writeEndElement();
              }
            }
            xml.writeEndElement();
          }
          else
          {
            if ("MeshOutputType" == lines[i])
            {
              xml.writeStartElement("Data");
              xml.writeAttribute("type",  "MeshData");
              xml.writeAttribute("bounds", bounds[output].toString());
              xml.writeEndElement();
            }
          }
        }
        xml.writeEndElement();
      }
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    m_storage->saveSnapshot(SnapshotData(outputsFile, buffer));
  }
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::reportProgress(unsigned int progress) const
{
  if (m_reporter) m_reporter->setProgress(progress);
}

//-----------------------------------------------------------------------------
SegFile_V4::SegFile_V4()
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V4::load(QuaZip&           zip,
                              CoreFactorySPtr   factory,
                              ProgressReporter *reporter,
                              ErrorHandlerSPtr  handler,
                              const LoadOptions options)
{
  Loader loader(zip, factory, reporter, handler, options);

  return loader.load();
}

//-----------------------------------------------------------------------------
void SegFile_V4::save(AnalysisPtr      analysis,
                      QuaZip&          zip,
                      ProgressReporter *reporter,
                      ErrorHandlerSPtr handler)
{
  Q_ASSERT(false);
}
