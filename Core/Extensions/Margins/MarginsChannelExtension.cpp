/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MarginsChannelExtension.h"

#include "MarginDetector.h"
#include "MarginsSegmentationExtension.h"

#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"

#include <vtkDistancePolyDataFilter.h>
#include <vtkPointData.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkContourFilter.h>
#include <vtkReverseSense.h>
#include <vtkPolyDataNormals.h>
#include <vtkRuledSurfaceFilter.h>
#include <vtkLine.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>

#include <QApplication>
#include <QDebug>
#include <QMessageBox>

typedef ModelItem::ArgumentId ArgumentId;

const ModelItemExtension::ExtId MarginsChannelExtension::ID = "MarginsExtension";

const ModelItemExtension::InfoTag MarginsChannelExtension::LEFT_MARGIN   = "Left Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::TOP_MARGIN    = "Top Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::UPPER_MARGIN  = "Upper Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::RIGHT_MARGIN  = "Right Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::BOTTOM_MARGIN = "Bottom Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::LOWER_MARGIN  = "Lower Margin";

const ArgumentId MarginsChannelExtension::MARGINTYPE = "MarginType";

//-----------------------------------------------------------------------------
MarginsChannelExtension::MarginsChannelExtension()
: m_useChannelBounds(true)
{
  for (int face = 0; face < 6; face++)
    m_PolyDataFaces[face] = NULL;
//   m_availableInformations << LEFT_MARGIN;
//   m_availableInformations << TOP_MARGIN;
//   m_availableInformations << UPPER_MARGIN;
//   m_availableInformations << RIGHT_MARGIN;
//   m_availableInformations << BOTTOM_MARGIN;
//   m_availableInformations << LOWER_MARGIN;
}

//-----------------------------------------------------------------------------
MarginsChannelExtension::~MarginsChannelExtension()
{

}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtId MarginsChannelExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void MarginsChannelExtension::initialize(ModelItem::Arguments args)
{
  ModelItem::Arguments extArgs(args.value(ID, QString()));

  bool computeMargin = false;
  if (extArgs.contains(MARGINTYPE))
  {
    computeMargin = extArgs[MARGINTYPE] == "Yes";
  } else
  {
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QMessageBox msgBox;
    msgBox.setWindowTitle("Margins Channel Extension");
    msgBox.setText(tr("Compute %1's margins").arg(m_channel->data().toString()));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    computeMargin = msgBox.exec() == QMessageBox::Yes;
    extArgs[MARGINTYPE] = computeMargin?"Yes":"No";
    QApplication::restoreOverrideCursor();
  }

  if (computeMargin)
    computeMargins();

  m_init = true;
  m_useChannelBounds = !computeMargin;
  m_args = extArgs;
}

//-----------------------------------------------------------------------------
void MarginsChannelExtension::computeMargins()
{
  m_borders = vtkSmartPointer<vtkPolyData>::New();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  MarginDetector *marginDetector = new MarginDetector(this);
  connect(marginDetector, SIGNAL(finished()),
          marginDetector, SLOT(deleteLater()));
  marginDetector->start();
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
QString MarginsChannelExtension::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
QVariant MarginsChannelExtension::information(ModelItemExtension::InfoTag tag) const
{
  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
ChannelExtension* MarginsChannelExtension::clone()
{
  return new MarginsChannelExtension();
}

typedef std::pair<unsigned int, unsigned long int> ComputedSegmentation;

//-----------------------------------------------------------------------------
void MarginsChannelExtension::computeMarginDistance(Segmentation* seg)
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

  ModelItemExtension *ext = seg->extension(ID);
  Q_ASSERT(ext);
  MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext);
  Q_ASSERT(marginExt);
  Nm distance[6], smargins[6];
  seg->volume()->bounds(smargins);
  if (m_useChannelBounds)
  {
    double cmargins[6];
    m_channel->volume()->bounds(cmargins);
    for (int i = 0; i < 6; i++)
      distance[i] = fabs(smargins[i] - cmargins[i]);
  }
  else
  {
    if (NULL == m_PolyDataFaces[0])
      ComputeSurfaces();

    for(int face = 0; face < 6; face++)
    {
      vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
      distanceFilter->SignedDistanceOff();
      distanceFilter->SetInputConnection( 0,  seg->mesh());
      distanceFilter->SetInputConnection( 1, m_PolyDataFaces[face]->GetProducerPort());
      distanceFilter->Update();
      distance[face] = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
    }
  }
  marginExt->setMargins(distance);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> MarginsChannelExtension::margins()
{
  // Ensure Margin Detector's finished
  if (m_borders.GetPointer() == NULL)
    computeMargins();

  m_borderMutex.lock();
  Q_ASSERT(m_borders.GetPointer() != NULL);
  m_borderMutex.unlock();
  return m_borders;
}

//-----------------------------------------------------------------------------
Nm MarginsChannelExtension::computedVolume()
{
  // Ensure Margin Detector's finished
  if (m_borders.GetPointer() == NULL)
    computeMargins();

  m_borderMutex.lock();
  m_borderMutex.unlock();
  return m_computedVolume;
}


//-----------------------------------------------------------------------------
void MarginsChannelExtension::ComputeSurfaces(void)
{
  m_borderMutex.lock();

  vtkPoints *borderPoints = m_borders->GetPoints();
  int sliceNum = m_borders->GetNumberOfPoints()/4;

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
  m_borderMutex.unlock();
}
