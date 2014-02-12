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

#include "SegFile.h"
#include "ReadOnlyFilter.h"
#include <Core/Utils/TemporalStorage.h>
#include "ClassificationXML.h"
#include "FetchRawData.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Graph/DirectedGraph.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Query.h>
#include <Core/Factory/CoreFactory.h>

using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;
using namespace EspINA::IO::Graph;

const QString SegFile::SegFile_V4::FORMAT_INFO_FILE = "settings.ini";

const QString TRACE_FILE          = "trace.dot";
const QString CLASSIFICATION_FILE = "taxonomy.xml";
const QString FILE_VERSION        = "version"; //backward compatibility

//-----------------------------------------------------------------------------
SegFile_V4::SegFile_V4()
: m_fetchBehaviour{new FetchRawData()}
{

}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile_V4::load(QuaZip&          zip,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler)
{
  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  m_storage = TemporalStorageSPtr{new TemporalStorage(tmpDir)};

  m_analysis = AnalysisSPtr{new Analysis()};
  m_factory  = factory;
  m_handler  = handler;

  m_vertexUuids.clear();
  m_trace.reset();
  m_loadedVertices.clear();

  if (!zip.setCurrentFile(CLASSIFICATION_FILE))
  {
    if (handler)
      handler->error(QObject::tr("Could not load analysis classification"));

    throw (Classification_Not_Found_Exception());
  }

  try
  {
    auto classification = ClassificationXML::parse(readCurrentFileFromZip(zip, handler));
    m_analysis->setClassification(classification);
  } catch (ClassificationXML::Parse_Exception e)
  {
    if (handler)
      handler->error(QObject::tr("Error while loading classification"));

    throw (Parse_Exception());
  }

  loadTrace(zip);

  QMap<QString, QList<QByteArray>> trcFiles;

  bool hasFile = zip.goToFirstFile();
  while (hasFile)
  {
    QString file = zip.getCurrentFileName();

    if (file != FORMAT_INFO_FILE
     && file != CLASSIFICATION_FILE
     && file != TRACE_FILE)
    {
      // Translate filenames to expected format
      if (file.contains("Outputs/"))
      {
        auto trcFile = file.remove(0, 8);
        auto vertex  = trcFile.split("_")[0].toInt();
        auto uuid    = m_filerUuids[vertex];
        auto outputs = QString("Filters/%1/outputs.xml").arg(uuid.toString());

        trcFiles[outputs].append(readCurrentFileFromZip(zip, handler));
      } else if (file.contains("SegmentationVolume/"))
      {
        auto oldFile = file.remove(0, 19);
        auto parts   = oldFile.split("_");
        auto vertex  = parts[0].toInt();
        auto uuid    = m_filerUuids[vertex];
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

        m_storage->saveSnapshot(SnapshotData(newFile, readCurrentFileFromZip(zip, handler)));
      } else {
        m_storage->saveSnapshot(SnapshotData(file, readCurrentFileFromZip(zip, handler)));
      }
    }

    hasFile = zip.goToNextFile();
  }

  for(auto filter : trcFiles.keys())
  {
    QByteArray buffer;
    QXmlStreamWriter xml(&buffer);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    int id = 0;
    for(auto trc : trcFiles[filter])
    {
      xml.writeStartElement("Output");
      xml.writeAttribute("id", QString::number(id));

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
      id++;
    }

    xml.writeEndDocument();

    m_storage->saveSnapshot(SnapshotData(filter, buffer));
  }

  return m_analysis;
}

//-----------------------------------------------------------------------------
void SegFile_V4::save(AnalysisPtr      analysis,
                      QuaZip&          zip,
                      ErrorHandlerSPtr handler)
{
  Q_ASSERT(false);
}


//-----------------------------------------------------------------------------
struct Vertex_Not_Found_Exception{};

//-----------------------------------------------------------------------------
PersistentSPtr SegFile_V4::findVertex(int id) const
{
  for(DirectedGraph::Vertex vertex : m_loadedVertices)
  {
    if (vertex->uuid() == m_vertexUuids[id]) return vertex;
  }

  throw Vertex_Not_Found_Exception();
}

//-----------------------------------------------------------------------------
SampleSPtr SegFile_V4::createSample(DirectedGraph::Vertex roVertex)
{
  SampleSPtr sample = m_factory->createSample();

  sample->setName(roVertex->name());
  sample->restoreState(roVertex->state());
  sample->setStorage(m_storage);

  m_analysis->add(sample);

  return sample;
}

//-----------------------------------------------------------------------------
FilterSPtr SegFile_V4::createFilter(DirectedGraph::Vertex roVertex)
{
  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex);

  OutputSList inputs;
  for(auto edge : inputConections)
  {
    auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
    auto input     = findVertex(vertex_v4->vertexId());

    FilterSPtr filter = std::dynamic_pointer_cast<Filter>(input);
    if (filter)
    {
      Output::Id id = atoi(edge.relationship.c_str());

      inputs << filter->output(id);
    }
  }

  FilterSPtr filter;
  try
  {
    filter = m_factory->createFilter(inputs, roVertex->name());
  } catch (CoreFactory::Unknown_Type_Exception e)
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
      m_filerUuids[parts[1].toInt()] = filter->uuid();
    }
  }

  return filter;
}

//-----------------------------------------------------------------------------
QPair<FilterSPtr, Output::Id> SegFile_V4::findOutput(DirectedGraph::Vertex   roVertex,
                                                     const QString          &linkName)
{
  QPair<FilterSPtr, Output::Id> output;

  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex, linkName);
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
  auto input     = findVertex(vertex_v4->vertexId());

  output.first  = std::dynamic_pointer_cast<Filter>(input);
  output.second = 0;//atoi(edge.relationship.c_str());

  return output;
}

//-----------------------------------------------------------------------------
ChannelSPtr SegFile_V4::createChannel(DirectedGraph::Vertex   roVertex)
{
  DirectedGraph::Edges inputConections = m_trace->inEdges(roVertex, "Volume");
  Q_ASSERT(inputConections.size() == 1);

  DirectedGraph::Edge edge = inputConections.first();

  auto vertex_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
  auto filter = std::dynamic_pointer_cast<Filter>(findVertex(vertex_v4->vertexId()));

  ChannelSPtr channel = m_factory->createChannel(filter, 0);


  qDebug() << roVertex->state();
  qDebug() << vertex_v4->state();
  channel->setName(roVertex->name());
  channel->restoreState(roVertex->state() + vertex_v4->state());
  channel->setStorage(m_storage);

  m_analysis->add(channel);

  return channel;
}

//-----------------------------------------------------------------------------
QString SegFile_V4::parseCategoryName(const State& state)
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
SegmentationSPtr SegFile_V4::createSegmentation(DirectedGraph::Vertex   roVertex)
{
  auto output = findOutput(roVertex, "CreateSegmentation");

  SegmentationSPtr segmentation = m_factory->createSegmentation(output.first, output.second);

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
void SegFile_V4::loadTrace(QuaZip& zip)
{
  m_trace = DirectedGraphSPtr(new DirectedGraph());

  QTextStream textStream(readFileFromZip(TRACE_FILE, zip, m_handler));

  std::istringstream stream(textStream.readAll().toStdString().c_str());
  read(stream, m_trace);

  for(DirectedGraph::Vertex roVertex : m_trace->vertices())
  {
    ReadOnlyVertex *rov = dynamic_cast<ReadOnlyVertex *>(roVertex.get());

    PersistentSPtr vertex;
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
        vertex = createSegmentation(roVertex);
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

  for(auto edge : m_trace->edges())
  {
    auto source_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.source);
    PersistentSPtr source = findVertex(source_v4->vertexId());

    auto target_v4 = std::dynamic_pointer_cast<ReadOnlyVertex>(edge.target);
    PersistentSPtr target = findVertex(target_v4->vertexId());

    if (source_v4->type() != VertexType::FILTER && target_v4->type() != VertexType::FILTER)
    {
      std::string relation = edge.relationship;
      try
      {
        if (source_v4->type() == VertexType::SAMPLE && target_v4->type() == VertexType::SEGMENTATION && relation == "where")
        {
          relation = Query::CONTAINS.toStdString();
        }
        m_analysis->addRelation(source, target, relation.c_str());
      } catch (...)
      {
        qWarning() << "Invalid Relationship: " << relation.c_str();
      }
    }
  }
}