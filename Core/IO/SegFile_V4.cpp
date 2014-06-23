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

#include "SegFile_V4.h"

#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Graph/DirectedGraph.h"
#include "Core/Analysis/Persistent.h"
#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Query.h"
#include "Core/Factory/CoreFactory.h"
#include "Core/IO/SegFile.h"
#include "Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h"
#include "Core/IO/ClassificationXML.h"
#include "Core/IO/ReadOnlyFilter.h"
#include "Core/Utils/TemporalStorage.h"

using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;
using namespace EspINA::IO::Graph;

const QString SegFile::SegFile_V4::FORMAT_INFO_FILE = "settings.ini";

const QString TRACE_FILE          = "trace.dot";
const QString CLASSIFICATION_FILE = "taxonomy.xml";
const QString FILE_VERSION        = "version"; //backward compatibility

//-----------------------------------------------------------------------------
SegFile_V4::Loader::Loader(QuaZip& zip, CoreFactorySPtr factory, ErrorHandlerSPtr handler)
: m_zip(zip)
, m_factory(factory)
, m_handler(handler)
//, m_fetchBehaviour{new FetchRawData()}
, m_fetchBehaviour{new MarchingCubesFromFetchedVolumetricData()}
, m_analysis{new Analysis()}
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V4::Loader::load()
{
  m_storage = TemporalStorageSPtr{new TemporalStorage()};
  m_analysis->setStorage(m_storage);

  m_vertexUuids.clear();
  m_trace.reset();
  m_loadedVertices.clear();

  if (!m_zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (m_handler)
      m_handler->error(QObject::tr("Could not load analysis classification"));

    throw (Classification_Not_Found_Exception());
  }

  try
  {
    auto currentFile    = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
    auto classification = ClassificationXML::parse(currentFile, m_handler);
    m_analysis->setClassification(classification);
  } catch (ClassificationXML::Parse_Exception &e)
  {
    if (m_handler)
      m_handler->error(QObject::tr("Error while loading classification"));

    throw (Parse_Exception());
  }

  loadTrace();

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    QString file = m_zip.getCurrentFileName();

    if ( file != FORMAT_INFO_FILE
      && file != CLASSIFICATION_FILE
      && file != TRACE_FILE
      && !file.contains("Outputs/")
      && !file.contains("SegmentationVolume/"))
    {
      auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
      m_storage->saveSnapshot(SnapshotData(file, currentFile));
    }

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
  for(DirectedGraph::Vertex vertex : m_loadedVertices)
  {
    if (vertex->uuid() == m_vertexUuids[id]) return vertex;
  }

  return DirectedGraph::Vertex();
}

//-----------------------------------------------------------------------------
SampleSPtr SegFile_V4::Loader::createSample(DirectedGraph::Vertex roVertex)
{
  SampleSPtr sample = m_factory->createSample();

  sample->setName(roVertex->name());
  sample->restoreState(roVertex->state());
  sample->setStorage(m_storage);

  m_analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V4::Loader::createFilter(DirectedGraph::Vertex roVertex)
{
  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex);

  InputSList inputs;
  for(auto edge : inputConections)
  {
    auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
    auto input     = inflateVertexV4(vertex_v4);

    FilterSPtr inputFilter = std::dynamic_pointer_cast<Filter>(input);
    if (inputFilter)
    {
      Output::Id id = atoi(edge.relationship.c_str());

      // Here it is safe to create the outputs because they already existed
      // In addition, it may be the case an update couldn't be executed if
      // traceability was disabled
      if (!inputFilter->m_outputs.contains(id))
      {
        inputFilter->m_outputs[id] = OutputSPtr{new Output(inputFilter.get(), id)};
      }

      inputs << getInput(inputFilter, id);
    }
  }

  FilterSPtr filter;
  try
  {
    filter = m_factory->createFilter(inputs, roVertex->name());
  } catch (...)
  {
    filter = FilterSPtr{new ReadOnlyFilter(inputs, roVertex->name())};
    filter->setFetchBehaviour(m_fetchBehaviour);
  }
  filter->setErrorHandler(m_handler);
  filter->setName(roVertex->name());
  filter->restoreState(roVertex->state());
  filter->setStorage(m_storage);

  State state  = roVertex->state();
  for(auto arg : state.split(";"))
  {
    auto parts = arg.split("=");
    if ("ID" == parts[0])
    {
      createFilterOutputsFile(filter, parts[1].toInt());
    }
  }

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V4::Loader::findOutput(DirectedGraph::Vertex   roVertex,
                                                             const QString          &linkName)
{
  QPair<FilterSPtr, Output::Id> output;

  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex, linkName);
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
  auto input     = inflateVertexV4(vertex_v4);

  output.first  = std::dynamic_pointer_cast<Filter>(input);
  output.second = parseOutputId(roVertex->state());

  return output;
}

//-----------------------------------------------------------------------------
ChannelSPtr SegFile_V4::Loader::createChannel(DirectedGraph::Vertex roVertex)
{
  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex, "Volume");
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  auto vertex_v4 = edge.source;
  auto filter    = std::dynamic_pointer_cast<Filter>(inflateVertexV4(vertex_v4));

  filter->update(0); // Existing outputs weren't stored in previous versions

  ChannelSPtr channel = m_factory->createChannel(filter, 0);

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

  QStringList params = state.split(";");

  for (auto param : params)
  {
    auto tokens = param.split("=");
    if ("Taxonomy" == tokens[0])
      category = tokens[1];
  }

  return category;
}

//-----------------------------------------------------------------------------
int SegFile_V4::Loader::parseOutputId(const State& state)
{
  int id = 0;

  QStringList params = state.split(";");

  for (auto param : params)
  {
    auto tokens = param.split("=");
    if ("Output" == tokens[0])
      id = tokens[1].toInt();
  }

  return id;
}

//-----------------------------------------------------------------------------
SegmentationSPtr SegFile_V4::Loader::createSegmentation(DirectedGraph::Vertex roVertex)
{
  auto roOutput = findOutput(roVertex, "CreateSegmentation");

  auto filter   = roOutput.first;
  auto outputId = roOutput.second;

  filter->update(outputId); // Existing outputs weren't stored in previous versions

  SegmentationSPtr segmentation = m_factory->createSegmentation(filter, outputId);

  State roState = roVertex->state();
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
  m_trace = DirectedGraphSPtr(new DirectedGraph());

  QTextStream textStream(readFileFromZip(TRACE_FILE, m_zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, m_trace);

  for(DirectedGraph::Vertex roVertex : m_trace->vertices())
  {
    inflateVertexV4(roVertex);
  }
}

//-----------------------------------------------------------------------------
DirectedGraph::Vertex SegFile_V4::Loader::inflateVertexV4(DirectedGraph::Vertex roVertex)
{
  ReadOnlyVertex *rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());

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
        vertex = createChannel(roVertex);
        break;
      }
      case VertexType::FILTER:
      {
        vertex = createFilter(roVertex);
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
  for (auto roVertex : m_pendingSegmenationVertices)
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
      } catch (...)
      {
        qWarning() << "Invalid Relationship: " << relation.c_str();
      }
    }
  }
}

//-----------------------------------------------------------------------------
void SegFile_V4::Loader::createFilterOutputsFile(FilterSPtr filter, int filterVertex)
{
  QString outputsFile;
  QMap<int, QList<QByteArray>> trcFiles;

  const QUuid uuid = filter->uuid();

  bool hasFile = m_zip.goToFirstFile();
  while (hasFile)
  {
    QString file = m_zip.getCurrentFileName();

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

      } else if (file.contains("SegmentationVolume/"))
      {
        auto oldFile = file.remove(0, 19);
        auto parts   = oldFile.split("_");
        auto vertex  = parts[0].toInt();
        if (vertex == filterVertex)
        {
          auto newFile = QString("Filters/%1/").arg(uuid.toString());
          if (parts[1].endsWith(".mhd"))
          {
            newFile += QString("VolumetricData_%1").arg(parts[1]);
          } else if (parts[1].endsWith(".raw"))
          {
            newFile += oldFile; // mhd internal references to raw files are not modified
          } else {
            Q_ASSERT(false);
          }

          auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
          m_storage->saveSnapshot(SnapshotData(newFile, currentFile));
        }
      } else if (file.contains("MeshOutputType/"))
      {
        auto oldFile = file.remove(0, 15);
        auto parts   = oldFile.split("_");
        auto vertex  = parts[0].toInt();
        if (vertex == filterVertex)
        {
          auto newFile = QString("Filters/%1/").arg(uuid.toString());
          Q_ASSERT(parts[1].endsWith(".vtp"));
          auto strings = parts[1].split("-");
          newFile += QString("MeshData_%1.vtp").arg(strings[0]);

          auto currentFile = SegFileInterface::readCurrentFileFromZip(m_zip, m_handler);
          m_storage->saveSnapshot(SnapshotData(newFile, currentFile));
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
        xml.writeAttribute("id", QString::number(output));

        QString content(trc);

        QStringList lines = content.split(QRegExp("[\r\n]"));
        for(int i = 0; i < lines.size(); ++i)
        {
          if ("SegmentationVolume" == lines[i])
          {
            xml.writeStartElement("Data");
            xml.writeAttribute("type", "VolumetricData");
            xml.writeEndElement();
          } else if ("MeshOutputType" == lines[i])
          {
            xml.writeStartElement("Data");
            xml.writeAttribute("type", "MeshData");
            xml.writeEndElement();
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
SegFile_V4::SegFile_V4()
{
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V4::load(QuaZip&          zip,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler)
{
  Loader loader(zip, factory, handler);

  return loader.load();
}

//-----------------------------------------------------------------------------
void SegFile_V4::save(AnalysisPtr      analysis,
                      QuaZip&          zip,
                      ErrorHandlerSPtr handler)
{
  Q_ASSERT(false);
}
