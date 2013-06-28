/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AdaptiveEdges.h"

#include "EdgeDetector.h"
#include "EdgeDistance.h"

#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Filter.h>
#include <Core/OutputRepresentations/MeshType.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkContourFilter.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataWriter.h>
#include <vtkReverseSense.h>
#include <vtkRuledSurfaceFilter.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkGenericDataObjectReader.h>

#include <QApplication>
#include <QDebug>
#include <QMessageBox>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;

const QString AdaptiveEdges::EXTENSION_FILE = "AdaptiveEdges/AdaptiveEdges.ext";
const QString AdaptiveEdges::EDGES_FILE     = "AdaptiveEdges/ChannelEdges";
const QString AdaptiveEdges::FACES_FILE     = "AdaptiveEdges/ChannelFaces";

AdaptiveEdges::ExtensionCache AdaptiveEdges::s_cache;

const ArgumentId AdaptiveEdges::EDGETYPE = "MarginType";

const std::string FILE_VERSION = AdaptiveEdgesID.toStdString() + " 1.0\n";
const char SEP = ',';

//-----------------------------------------------------------------------------
AdaptiveEdges::AdaptiveEdges(bool useAdaptiveEdges,
                             int backgroundColor,
                             int threshold)
: m_useAdaptiveEdges(useAdaptiveEdges)
, m_backgroundColor(backgroundColor)
, m_threshold(threshold)
{
}

//-----------------------------------------------------------------------------
AdaptiveEdges::~AdaptiveEdges()
{
  if (m_channel)
    invalidate();
}

//-----------------------------------------------------------------------------
ModelItem::ExtId AdaptiveEdges::id()
{
  return AdaptiveEdgesID;
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeAdaptiveEdges()
{
  //qDebug() << "Computing Adaptive Edges" << m_channel->data().toString() << AdaptiveEdgesID;
  ExtensionData &data = s_cache[m_channel].Data;

  data.Edges = vtkSmartPointer<vtkPolyData>::New();
  data.UseAdaptiveEdges = true;
  data.BackgroundColor  = m_backgroundColor;
  data.Threshold        = m_threshold;

  EdgeDetector *marginDetector = new EdgeDetector(this);
  connect(marginDetector, SIGNAL(finished()),
          marginDetector, SLOT(deleteLater()));
  marginDetector->start();
}

typedef vtkSmartPointer<vtkGenericDataObjectReader> VTKReader;

//-----------------------------------------------------------------------------
void AdaptiveEdges::loadEdgesCache(ChannelPtr channel)
{
  ExtensionData &data = s_cache[channel].Data;

  m_mutex.lock();
  if (data.Edges.GetPointer() == NULL && !channel->outputIsModified())
  {
    QString edgesFile = QString(EDGES_FILE + "%1.vtp").arg(fileId(channel));
    QFileInfo file(channel->filter()->cacheDir().absoluteFilePath(edgesFile));

    if (file.exists())
    {
      VTKReader reader = VTKReader::New();
      reader->SetFileName(file.absoluteFilePath().toUtf8());
      reader->SetReadAllFields(true);
      reader->Update();

      data.Edges = vtkSmartPointer<vtkPolyData>(reader->GetPolyDataOutput());
    }
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::loadFacesCache(ChannelPtr channel)
{
  ExtensionData &data = s_cache[channel].Data;

  if (data.Faces[0].GetPointer() == NULL && !channel->outputIsModified())
  {
    for (int i = 0; i < 6; ++i)
    {
      QString edgesFile = QString(FACES_FILE + "%1_%2.vtp").arg(fileId(channel)).arg(i);
      QFileInfo file(channel->filter()->cacheDir().absoluteFilePath(edgesFile));

      if (file.exists())
      {
        VTKReader reader = VTKReader::New();
        reader->SetFileName(file.absoluteFilePath().toUtf8());
        reader->SetReadAllFields(true);
        reader->Update();

        data.Faces[i] = vtkSmartPointer<vtkPolyData>(reader->GetPolyDataOutput());
      }
    }
  }
}

//-----------------------------------------------------------------------------
ChannelPtr AdaptiveEdges::findChannel(const QString &id,
                                      int outputId,
                                      const QDir &tmpDir,
                                      IEspinaModel *model)
{
  ChannelPtr extensionChannel = NULL;

  int i = 0;
  while (!extensionChannel && i < model->channels().size())
  {
    ChannelSPtr channel = model->channels()[i];
    if ( channel->filter()->id()       == id
      && channel->outputId()           == outputId
      && channel->filter()->cacheDir() == tmpDir)
    {
      extensionChannel = channel.get();
    }
    i++;
  }

  return extensionChannel;
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::loadCache(QuaZipFile &file,
                              const QDir &tmpDir,
                              IEspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
    char buffer[1024];
    while (file.readLine(buffer, sizeof(buffer)) > 0)
    {
      QString line(buffer);
      QStringList fields = line.split(SEP);

      ChannelPtr extensionChannel = findChannel(fields[0], fields[1].toInt(), tmpDir, model);
      if (extensionChannel)
      {
        ExtensionData &data = s_cache[extensionChannel].Data;

        data.UseAdaptiveEdges = fields[2].toInt();
        // First versions didn't stored the computed volume
        if (fields.size() > 3)
        {
          data.ComputedVolume   = fields[3].toDouble();

          // older versions didn't store background and threshold
          if (fields.size() > 4)
          {
            data.BackgroundColor  = fields[4].toInt();
            data.Threshold        = fields[5].toInt();
          }
        }

      }
      else 
      {
        qWarning() << AdaptiveEdgesID << "Invalid Cache Entry:" << line;
      }
    }
  }
}

typedef vtkSmartPointer<vtkGenericDataObjectWriter> VTKWriter;

//-----------------------------------------------------------------------------
bool AdaptiveEdges::saveCache(Snapshot &snapshot)
{
  s_cache.purge();

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  ChannelPtr channel;
  foreach(channel, s_cache.keys())
  {
    ExtensionData &data = s_cache[channel].Data;

    QString channelId = channel->filter()->id();

    cache << channelId.toStdString();
    cache << SEP << channel->outputId();

    cache << SEP << data.UseAdaptiveEdges;
    cache << SEP << data.ComputedVolume;
    cache << SEP << data.BackgroundColor;
    cache << SEP << data.Threshold;

    cache << std::endl;

    VTKWriter polyWriter = VTKWriter::New();

    if (data.UseAdaptiveEdges)
    {
      loadEdgesCache(channel);
      if (data.Edges)
      {
        channelId += QString("-%1").arg(channel->outputId());
        polyWriter->SetInputConnection(data.Edges->GetProducerPort());
        polyWriter->SetFileTypeToBinary();
        polyWriter->WriteToOutputStringOn();
        polyWriter->Write();

        QByteArray  edges(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
        snapshot << SnapshotEntry(QString(EDGES_FILE + "%1.vtp").arg(channelId), edges);
      }

      loadFacesCache(channel);
      for(int i = 0; i < 6; i++)
      {
        if (data.Faces[i])
        {
          polyWriter->SetInputConnection(data.Faces[i]->GetProducerPort());
          polyWriter->Update();
          polyWriter->Write();

          QByteArray face(polyWriter->GetOutputString(), polyWriter->GetOutputStringLength());
          snapshot << SnapshotEntry(QString(FACES_FILE + "%1_%2.vtp").arg(channelId).arg(i), face);
        }
      }
    }
  }

  snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());


  return true;
}

//-----------------------------------------------------------------------------
Channel::ExtensionPtr AdaptiveEdges::clone()
{
  return new AdaptiveEdges();
}

typedef std::pair<unsigned int, unsigned long int> ComputedSegmentation;

//-----------------------------------------------------------------------------
void AdaptiveEdges::initialize()
{
  if (s_cache.isCached(m_channel))
  {
    s_cache.markAsClean(m_channel);

    m_useAdaptiveEdges = s_cache[m_channel].Data.UseAdaptiveEdges;
  } else  if (m_useAdaptiveEdges)
  {
    computeAdaptiveEdges();
  }
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::invalidate(ChannelPtr channel)
{
  if (!channel)
    channel = m_channel;

  if (s_cache.isCached(channel))
  {
    //qDebug() << "Invalidate" << channel->data().toString();
    s_cache.markAsDirty(channel);
  }
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeDistanceToEdge(SegmentationPtr seg)
{
  Segmentation::InformationExtension ext = seg->informationExtension(EdgeDistanceID);
  Q_ASSERT(ext);
  EdgeDistance *distanceExt = edgeDistancePtr(ext);

  SegmentationVolumeSPtr segVolume = segmentationVolume(seg->output());
  Nm distance[6], segMargins[6];
  segVolume->bounds(segMargins);

  ExtensionData &data = s_cache[m_channel].Data;
  if (m_useAdaptiveEdges)
  {
    loadFacesCache(m_channel);

    if (NULL == data.Faces[0])
      ComputeSurfaces();

    for(int face = 0; face < 6; face++)
    {
      vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
      distanceFilter->SignedDistanceOff();
      distanceFilter->SetInputConnection( 0, meshRepresentation(seg->output())->mesh());
      distanceFilter->SetInputConnection( 1, data.Faces[face]->GetProducerPort());
      distanceFilter->Update();
      distance[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    }
  }
  else
  {
    Nm channelMargins[6];
    m_channel->volume()->bounds(channelMargins);
    for (int i = 0; i < 6; i+=2)
      distance[i] = segMargins[i] - channelMargins[i];
    for (int i = 1; i < 6; i+=2)
      distance[i] = channelMargins[i] - segMargins[i];
  }

  distanceExt->setDistances(distance);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AdaptiveEdges::channelEdges()
{
  loadEdgesCache(m_channel);

  ExtensionData &data = s_cache[m_channel].Data;
  // Ensure Margin Detector's finished
  if (data.Edges.GetPointer() == NULL)
    computeAdaptiveEdges();

  m_mutex.lock();
  Q_ASSERT(NULL != data.Edges.GetPointer());
  m_mutex.unlock();

  return data.Edges;
}

//-----------------------------------------------------------------------------
Nm AdaptiveEdges::computedVolume()
{
  Nm volume = 0;

  loadEdgesCache(m_channel);

  ExtensionData &data = s_cache[m_channel].Data;
  // Ensure Margin Detector's finished
  if (NULL == data.Edges.GetPointer())
    computeAdaptiveEdges();

  m_mutex.lock();
  volume = data.ComputedVolume;
  m_mutex.unlock();

  return volume;
}


//-----------------------------------------------------------------------------
void AdaptiveEdges::ComputeSurfaces()
{

  loadEdgesCache(m_channel);

  ExtensionData &data = s_cache[m_channel].Data;

  if (NULL == data.Edges)
    computeAdaptiveEdges();

  m_mutex.lock();

  vtkPoints *borderPoints = data.Edges->GetPoints();
  int sliceNum = data.Edges->GetNumberOfPoints()/4;

  for (int face = 0; face < 6; face++)
  {
    vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> faceCells = vtkSmartPointer<vtkCellArray>::New();
    if (face < 4)
    {
      for (int i = 0; i < sliceNum; i++)
      {
        switch(face)
        {
          case 0: // LEFT
            facePoints->InsertNextPoint(borderPoints->GetPoint(4*i));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+1));
            break;
          case 1: // RIGHT
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+2));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+3));
            break;
          case 2: // TOP
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+1));
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+2));
            break;
          case 3: // BOTTOM
            facePoints->InsertNextPoint(borderPoints->GetPoint((4*i)+3));
            facePoints->InsertNextPoint(borderPoints->GetPoint(4*i));
            break;
          default:
            Q_ASSERT(FALSE);
            break;
        }

        if (i == 0)
          continue;

        vtkIdType corners[4];
        corners[0] = (i*2)-2;
        corners[1] = (i*2)-1;
        corners[2] = (i*2)+1;
        corners[3] = 2*i;
        faceCells->InsertNextCell(4,corners);
      }
    }
    else
    {
      vtkIdType corners[4];
      switch(face)
      {
        case 4: // UPPER
          corners[0] = facePoints->InsertNextPoint(borderPoints->GetPoint(0));
          corners[1] = facePoints->InsertNextPoint(borderPoints->GetPoint(1));
          corners[2] = facePoints->InsertNextPoint(borderPoints->GetPoint(2));
          corners[3] = facePoints->InsertNextPoint(borderPoints->GetPoint(3));
          break;
        case 5: // LOWER
          corners[0] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-4));
          corners[1] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-3));
          corners[2] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-2));
          corners[3] = facePoints->InsertNextPoint(borderPoints->GetPoint(borderPoints->GetNumberOfPoints()-1));
          break;
        default:
          Q_ASSERT(FALSE);
          break;
      }
      faceCells->InsertNextCell(4,corners);
    }

    vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(facePoints);
    poly->SetPolys(faceCells);

    data.Faces[face] = poly;
  }
  m_mutex.unlock();
}

//-----------------------------------------------------------------------------
QString AdaptiveEdges::fileId(ChannelPtr channel) const
{
  return QString("%1-%2").arg(channel->filter()->id()).arg(channel->outputId());
}


//-----------------------------------------------------------------------------
AdaptiveEdgesPtr EspINA::adaptiveEdgesPtr(Channel::ExtensionPtr extension)
{
  return dynamic_cast<AdaptiveEdgesPtr>(extension);
}
