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


#include "regions/RectangularBoundingRegion.h"

#include <extensions/CountingRegionChannelExtension.h>
#include "regions/vtkBoundingRegionSliceWidget.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include "vtkBoundingRegion3DWidget.h"


const QString RectangularBoundingRegion::ID = "RectangularBoundingRegion";

//-----------------------------------------------------------------------------
RectangularBoundingRegion::RectangularBoundingRegion(CountingRegionChannelExtension *channelExt,
                                                     Nm borders[6],
                                                     Nm inclusion[3],
                                                     Nm exclusion[3],
                                                     ViewManager *vm)
: BoundingRegion(channelExt, inclusion, exclusion, vm)
{
  memcpy(m_borders, borders, 6*sizeof(Nm));

  m_boundingRegion = vtkPolyData::New();
  updateBoundingRegion();
}


//-----------------------------------------------------------------------------
RectangularBoundingRegion::~RectangularBoundingRegion()
{
  m_channelExt->removeRegion(this);
  foreach(vtkAbstractWidget *w, m_widgets)
  {
    w->EnabledOff();
    w->RemoveAllObservers();
    w->Delete();
  }
  m_widgets.clear();

  if (m_boundingRegion)
    m_boundingRegion->Delete();
}

//-----------------------------------------------------------------------------
QVariant RectangularBoundingRegion::data(int role) const
{
  if (role == Qt::DisplayRole)
  {
    QString repName = QString(tr("Rectangular (%1,%2,%3,%4,%5,%6)"))
      .arg(left(),0,'f',2).arg(top(),0,'f',2).arg(upper(),0,'f',2)
      .arg(right(),0,'f',2).arg(bottom(),0,'f',2).arg(lower(),0,'f',2);
    return repName;
  }

  return BoundingRegion::data(role);
}

//-----------------------------------------------------------------------------
QString RectangularBoundingRegion::serialize() const
{
  return QString("%1=%2,%3,%4,%5,%6,%7")
         .arg(ID)
         .arg(left(),0,'f',2).arg(top(),0,'f',2).arg(upper(),0,'f',2)
         .arg(right(),0,'f',2).arg(bottom(),0,'f',2).arg(lower(),0,'f',2);
}



//-----------------------------------------------------------------------------
vtkAbstractWidget *RectangularBoundingRegion::createWidget()
{
  vtkBoundingRegion3DWidget *w = vtkBoundingRegion3DWidget::New();
  Q_ASSERT(w);
  w->SetBoundingRegion(m_boundingRegion);

  m_widgets << w;

  return w;
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::deleteWidget(vtkAbstractWidget* widget)
{
  widget->Off();
  widget->RemoveAllObservers();

  vtkBoundingRegionWidget *brw = dynamic_cast<vtkBoundingRegionWidget *>(widget);
  m_widgets.removeAll(brw);

  widget->Delete();
}

//-----------------------------------------------------------------------------
SliceWidget* RectangularBoundingRegion::createSliceWidget(PlaneType plane)
{
  vtkBoundingRegionSliceWidget *w = vtkBoundingRegionSliceWidget::New();
  Q_ASSERT(w);
  w->AddObserver(vtkCommand::EndInteractionEvent, this);
  w->SetPlane(plane);
  w->SetBoundingRegion(m_boundingRegion);

  m_widgets << w;

  return new SliceWidget(w);
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::setEnabled(bool enable)
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::updateBoundingRegion()
{
  vtkSmartPointer<vtkPoints> vertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();

  vtkIdType upperFace[4], leftFace[4], topFace[4];
  vtkIdType lowerFace[4], rightFace[4], bottomFace[4];

  Nm Left   = m_borders[0] + m_inclusion[0];
  Nm Top    = m_borders[2] + m_inclusion[1];
  Nm Upper  = m_borders[4] + m_inclusion[2];
  Nm Right  = m_borders[1] - m_exclusion[0];
  Nm Bottom = m_borders[3] - m_exclusion[1];
  Nm Lower  = m_borders[5] - m_exclusion[2];

    // Upper Inclusion Face
  upperFace[0] = vertex->InsertNextPoint(Left,  Bottom, Upper);
  upperFace[1] = vertex->InsertNextPoint(Left,  Top,    Upper);
  upperFace[2] = vertex->InsertNextPoint(Right, Top,    Upper);
  upperFace[3] = vertex->InsertNextPoint(Right, Bottom, Upper);
  faces->InsertNextCell(4, upperFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Lower Exclusion Face
  lowerFace[0] = vertex->InsertNextPoint(Left,  Bottom, Lower);
  lowerFace[1] = vertex->InsertNextPoint(Left,  Top,    Lower);
  lowerFace[2] = vertex->InsertNextPoint(Right, Top,    Lower);
  lowerFace[3] = vertex->InsertNextPoint(Right, Bottom, Lower);
  faces->InsertNextCell(4, lowerFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Left Inclusion Face
  leftFace[0] = upperFace[0];
  leftFace[1] = upperFace[1];
  leftFace[2] = lowerFace[1];
  leftFace[3] = lowerFace[0];
  faces->InsertNextCell(4, leftFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Right Exclusion Face
  rightFace[0] = upperFace[2];
  rightFace[1] = upperFace[3];
  rightFace[2] = lowerFace[3];
  rightFace[3] = lowerFace[2];
  faces->InsertNextCell(4, rightFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  // Top Inclusion Face
  topFace[0] = upperFace[1];
  topFace[1] = upperFace[2];
  topFace[2] = lowerFace[2];
  topFace[3] = lowerFace[1];
  faces->InsertNextCell(4, topFace);
  faceData->InsertNextValue(INCLUSION_FACE);

  // Bottom Exclusion Face
  bottomFace[0] = upperFace[3];
  bottomFace[1] = upperFace[0];
  bottomFace[2] = lowerFace[0];
  bottomFace[3] = lowerFace[3];
  faces->InsertNextCell(4, bottomFace);
  faceData->InsertNextValue(EXCLUSION_FACE);

  m_boundingRegion->SetPoints(vertex);
  m_boundingRegion->SetPolys(faces);
  vtkCellData *data = m_boundingRegion->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");

  m_totalVolume = (m_borders[1]-m_borders[0]+1)*
                  (m_borders[3]-m_borders[2]+1)*
                  (m_borders[5]-m_borders[4]+1);
  m_inclusionVolume = (Right-Left)*(Top-Bottom)*(Upper-Lower);
}