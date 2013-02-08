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

#include <QApplication>
#include <QDebug>
#include <QMessageBox>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;

const QString AdaptiveEdges::EXTENSION_FILE = "AdaptiveEdges/AdaptiveEdges.ext";
const QString AdaptiveEdges::EDGES_FILE     = "AdaptiveEdges/ChannelEdges.vtk";
const QString AdaptiveEdges::FACES_FILE     = "AdaptiveEdges/ChannelFaces.vtk";

QMap<ChannelPtr, AdaptiveEdges::CacheEntry> AdaptiveEdges::s_cache;

const ModelItem::ExtId AdaptiveEdges::ID = "AdaptiveEdges";

const ArgumentId AdaptiveEdges::EDGETYPE = "MarginType";

const std::string FILE_VERSION = AdaptiveEdges::ID.toStdString() + " 1.0\n";
const char SEP = ',';

//-----------------------------------------------------------------------------
AdaptiveEdges::AdaptiveEdges(bool computeEdges)
: m_computeAdaptiveEdges(computeEdges)
{
  for (int face = 0; face < 6; face++)
    m_PolyDataFaces[face] = NULL;

}

//-----------------------------------------------------------------------------
AdaptiveEdges::~AdaptiveEdges()
{
}

//-----------------------------------------------------------------------------
ModelItem::ExtId AdaptiveEdges::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::initialize(ModelItem::Arguments args)
{
  qDebug() << "Initialize" << m_channel->data().toString() << ID;
  ModelItem::Arguments extArgs(args.value(ID, QString()));

  if (extArgs.contains(EDGETYPE))
  {
    m_computeAdaptiveEdges = extArgs[EDGETYPE] == "Yes";

    m_args = extArgs;
  } else
  {
    m_args[EDGETYPE] = m_computeAdaptiveEdges?"Yes":"No";
  }

  if (m_computeAdaptiveEdges)
    computeAdaptiveEdges();

  m_init = true;
}

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeAdaptiveEdges()
{
  qDebug() << "Computing Adaptive Edges" << m_channel->data().toString() << ID;
  m_edges = vtkSmartPointer<vtkPolyData>::New();

  EdgeDetector *marginDetector = new EdgeDetector(this);
  connect(marginDetector, SIGNAL(finished()),
          marginDetector, SLOT(deleteLater()));
  marginDetector->start();

  s_cache[m_channel].UseAdaptiveEdges = true;
}

//-----------------------------------------------------------------------------
QString AdaptiveEdges::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
bool AdaptiveEdges::loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() != FILE_VERSION)
    return false;

  char buffer[1024];
  while (file.readLine(buffer, sizeof(buffer)) > 0)
  {
    QString line(buffer);
    QStringList fields = line.split(SEP);

    ChannelPtr extensionChannel = NULL;
    int i = 0;
    while (!extensionChannel && i < model->channels().size())
    {
      ChannelSPtr channel = model->channels()[i];
      if ( channel->filter()->id()       == fields[0]
        && channel->outputId()           == fields[1].toInt()
        && channel->filter()->cacheDir() == tmpDir)
      {
        extensionChannel = channel.data();
      }
      i++;
    }
    Q_ASSERT(extensionChannel);

    s_cache[extensionChannel].UseAdaptiveEdges = fields[2].toInt();
  }

  return true;
}


//-----------------------------------------------------------------------------
bool AdaptiveEdges::saveCache(ModelItem::Extension::CacheList &cacheList)
{
  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  ChannelPtr channel;
  foreach(channel, s_cache.keys())
  {
    cache << channel->filter()->id().toStdString();
    cache << SEP << channel->outputId();

    cache << SEP << s_cache[channel].UseAdaptiveEdges;

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

//   vtkSmartPointer<vtkPolyDataWriter> edgesWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
//   edgesWriter->WriteToOutputStringOn();
//   //edgesWriter->SetFileTypeToBinary();


  return true;
}

//-----------------------------------------------------------------------------
Channel::ExtensionPtr AdaptiveEdges::clone()
{
  return new AdaptiveEdges();
}

typedef std::pair<unsigned int, unsigned long int> ComputedSegmentation;

//-----------------------------------------------------------------------------
void AdaptiveEdges::computeDistanceToEdge(SegmentationPtr seg)
{
  std::map<unsigned int, unsigned long int>::iterator it = m_ComputedSegmentations.find(seg->number());
  if (it != m_ComputedSegmentations.end())
  {
    // using itkVolume()->MTime and not mesh()->MTime() to avoid triggering lazy computations
    if ((*it).second == seg->volume()->toITK()->GetMTime())
      return;

    m_ComputedSegmentations.erase(seg->number());
  }

  m_ComputedSegmentations.insert(ComputedSegmentation(seg->number(), seg->volume()->toITK()->GetMTime()));

  Segmentation::InformationExtension ext = seg->informationExtension(EdgeDistance::ID);
  Q_ASSERT(ext);
  EdgeDistance *distanceExt = edgeDistancePtr(ext);

  Nm distance[6], smargins[6];
  seg->volume()->bounds(smargins);
  if (m_computeAdaptiveEdges)
  {
    if (NULL == m_PolyDataFaces[0])
      ComputeSurfaces();

    for(int face = 0; face < 6; face++)
    {
      vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
      distanceFilter->SignedDistanceOff();
      distanceFilter->SetInputConnection( 0, seg->volume()->toMesh());
      distanceFilter->SetInputConnection( 1, m_PolyDataFaces[face]->GetProducerPort());
      distanceFilter->Update();
      distance[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    }
  } else
  {
    double cmargins[6];
    m_channel->volume()->bounds(cmargins);
    for (int i = 0; i < 6; i++)
      distance[i] = fabs(smargins[i] - cmargins[i]);
  }

  distanceExt->setDistances(distance);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> AdaptiveEdges::channelEdges()
{
  // Ensure Margin Detector's finished
  if (m_edges.GetPointer() == NULL)
    computeAdaptiveEdges();

  m_mutex.lock();
  Q_ASSERT(m_edges.GetPointer() != NULL);
  m_mutex.unlock();

  return m_edges;
}

//-----------------------------------------------------------------------------
Nm AdaptiveEdges::computedVolume()
{
  // Ensure Margin Detector's finished
  if (m_edges.GetPointer() == NULL)
    computeAdaptiveEdges();

  m_mutex.lock();
  m_mutex.unlock();
  return m_computedVolume;
}


//-----------------------------------------------------------------------------
void AdaptiveEdges::ComputeSurfaces()
{
  m_mutex.lock();

  vtkPoints *borderPoints = m_edges->GetPoints();
  int sliceNum = m_edges->GetNumberOfPoints()/4;

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

    m_PolyDataFaces[face] = poly;
  }
  m_mutex.unlock();
}


AdaptiveEdgesPtr EspINA::adaptiveEdgesPtr(Channel::ExtensionPtr extension)
{
  return dynamic_cast<AdaptiveEdgesPtr>(extension);
}
