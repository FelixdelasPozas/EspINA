/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "BrushPicker.h"

#include "common/model/Channel.h"
#include "common/gui/EspinaRenderView.h"
#include <ViewManager.h>
#include <FreeFormSource.h>
#include <EspinaRegions.h>
#include <BoundingBox.h>

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

#include <itkImageRegionIteratorWithIndex.h>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkCoordinate.h>
#include <vtkSphere.h>
#include <vtkImageResliceToColors.h>


typedef itk::ImageRegionIteratorWithIndex<EspinaVolume> VolumeIterator;
//-----------------------------------------------------------------------------
BrushPicker::BrushPicker(PickableItem* item)
: m_referenceItem(item)
, m_displayRadius(20)
, m_borderColor(Qt::blue)
, m_brushColor(Qt::blue)
, m_previewVisible(true)
, m_tracking(false)
, m_stroke(IPicker::WorldRegion::New())
, m_plane(AXIAL)
, m_radius(-1)
, m_preview(NULL)
{

}

//-----------------------------------------------------------------------------
bool BrushPicker::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (!m_tracking && QEvent::MouseButtonPress == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent *>(e);
    if (me->button() == Qt::LeftButton)
    {
      startStroke(me->pos(), view);
      return true;
    }
  } else if (m_tracking && QEvent::MouseMove == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    updateStroke(me->pos(), view);
    return true;
  }else if (m_tracking && QEvent::MouseButtonRelease == e->type())
  {
    stopStroke(view);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
void BrushPicker::setRadius(int radius)
{
  if (radius <= 0)
    m_displayRadius = 1;
  else if (radius > MAX_RADIUS)
    m_displayRadius = MAX_RADIUS;
  else
    m_displayRadius = radius;

  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushPicker::setBorderColor(QColor color)
{
  m_borderColor = color;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushPicker::setBrushColor(QColor color)
{
  m_brushColor = color;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushPicker::setReferenceItem(PickableItem* item)
{
  m_referenceItem = item;
  m_spacing = m_referenceItem->itkVolume()->GetSpacing();
}

//-----------------------------------------------------------------------------
void BrushPicker::setStrokeVisibility(bool visible)
{
  m_previewVisible = visible;
}

//-----------------------------------------------------------------------------
void BrushPicker::buildCursor()
{
  int width = 2*m_displayRadius;

  QPixmap pix(width, width);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setBrush(QBrush(m_brushColor));
  p.setPen(QPen(m_borderColor));

  p.drawEllipse(0, 0, width-1, width-1);
  Q_ASSERT(pix.hasAlpha());

  m_cursor = QCursor(pix);
}

//-----------------------------------------------------------------------------
void BrushPicker::createBrush(double brush[3], QPoint pos)
{
  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;

  double wPos[3];
  wPos[m_plane] = m_pBounds[2*m_plane];
  wPos[H] = m_LL[H] + pos.x()*m_worldSize[0]/m_windowSize[0];
  wPos[V] = m_UR[V] + pos.y()*m_worldSize[1]/m_windowSize[1];

  //qDebug() << QString("Center (%1,%2,%3)").arg(wPos[0]).arg(wPos[1]).arg(wPos[2]);
  for(int i=0; i < 3; i++)
    brush[i] = int(wPos[i]/m_spacing[i]+0.5)*m_spacing[i];
  //qDebug() << QString("Stroke Point (%1,%2,%3)").arg(brush[0]).arg(brush[1]).arg(brush[2]);
}


//-----------------------------------------------------------------------------
bool BrushPicker::validStroke(double brush[3])
{
  double brushBounds[6];
  double sRadius = (m_plane == SAGITTAL)?0:m_radius;
  double cRadius = (m_plane ==  CORONAL)?0:m_radius;
  double aRadius = (m_plane ==    AXIAL)?0:m_radius;

  brushBounds[0] = brush[0] - sRadius;
  brushBounds[1] = brush[0] + sRadius;
  brushBounds[2] = brush[1] - cRadius;
  brushBounds[3] = brush[1] + cRadius;
  brushBounds[4] = brush[2] - aRadius;
  brushBounds[5] = brush[2] + aRadius;

  BoundingBox previewBB(m_pBounds);
  BoundingBox brushBB(brushBounds);

  return previewBB.intersect(brushBB);
}

//-----------------------------------------------------------------------------
void BrushPicker::startStroke(QPoint pos, EspinaRenderView* view)
{
  if (!m_referenceItem)
    return;

  view->previewBounds(m_pBounds);

  if (m_pBounds[0] == m_pBounds[1])
    m_plane = SAGITTAL;
  else if (m_pBounds[2] == m_pBounds[3])
    m_plane = CORONAL;
  else if (m_pBounds[4] == m_pBounds[5])
    m_plane = AXIAL;
  else
    return; // Current implementation only works in 2D views

  m_tracking = true;

  memcpy(m_windowSize, view->renderWindow()->GetSize(), 2*sizeof(int));

  // Display bounds in world coordinates
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  vtkRenderer *renderer = view->mainRenderer();
  coords->SetViewport(renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0); //LL
  memcpy(m_LL,coords->GetComputedWorldValue(renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(m_UR,coords->GetComputedWorldValue(renderer),3*sizeof(double));

  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;

  m_worldSize[0] = fabs(m_UR[H] - m_LL[H]);
  m_worldSize[1] = fabs(m_UR[V] - m_LL[V]);

  m_radius = m_displayRadius*m_worldSize[0]/m_windowSize[0];

  double brush[3];
  createBrush(brush, pos);

  startPreview(view);

  if (validStroke(brush))
  {
    m_lastDot = pos;
    m_stroke->InsertNextPoint(brush);
    emit stroke(m_referenceItem, brush[0], brush[1], brush[2], m_radius, m_plane);
    updatePreview(brush, view);
  }
}

//-----------------------------------------------------------------------------
void BrushPicker::updateStroke(QPoint pos, EspinaRenderView* view)
{
  if (m_stroke->GetNumberOfPoints() > 0 && QLineF(m_lastDot, pos).length() < m_displayRadius/2.0)
    return;

  double brush[3];
  createBrush(brush, pos);

  if (validStroke(brush))
  {
    m_lastDot = pos;
    m_stroke->InsertNextPoint(brush);
    emit stroke(m_referenceItem, brush[0], brush[1], brush[2], m_radius, m_plane);
    updatePreview(brush, view);
  }
}

//-----------------------------------------------------------------------------
void BrushPicker::stopStroke(EspinaRenderView* view)
{
  if (m_stroke->GetNumberOfPoints() > 0)
    emit stroke(m_referenceItem, m_stroke, m_radius, m_plane);

  //qDebug() << "Stroke with" << m_stroke->GetNumberOfPoints() << "points";
  m_tracking = false;
  m_stroke->Reset();
  stopPreview(view);
}

//-----------------------------------------------------------------------------
void BrushPicker::startPreview(EspinaRenderView* view)
{
  if (!m_previewVisible)
    return;

  EspinaVolume::PointType origin;
  origin.Fill(0);
  //origin[m_plane] = m_pBounds[m_plane] - 10;
  EspinaVolume::SizeType size;
  for(int i=0; i<6; i++)
    size[i] = m_pBounds[i];

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), 1.0);
  m_lut->Modified();

  m_preview = EspinaVolume::New();
  m_preview->SetOrigin(origin);
  m_preview->SetRegions(BoundsToRegion(m_pBounds, m_spacing));
  m_preview->SetSpacing(m_spacing);
  m_preview->Allocate();
  m_preview->FillBuffer(0);
  m_preview->Update();

  itk2vtk = itk2vtkFilterType::New();
  itk2vtk->ReleaseDataFlagOn();
  itk2vtk->SetInput(m_preview);
  itk2vtk->Update();

  m_actor = vtkImageActor::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(itk2vtk->GetOutput()->GetProducerPort());
  m_actor->GetProperty()->SetLookupTable(m_lut);

  double pos[3];
  memset(pos, 0, 3*sizeof(double));
  int sign = ((m_plane == AXIAL) ? -1 : 1);
  pos[m_plane] = -m_pBounds[2*m_plane] + (sign*0.1);
  m_actor->SetPosition(pos);
  m_actor->Update();

  view->addPreview(m_actor);
}

//-----------------------------------------------------------------------------
void BrushPicker::updatePreview(double brush[3], EspinaRenderView* view)
{
  if (!m_previewVisible)
    return;

  //qDebug() << "Update Preview" << brush[0] << brush[1] << brush[2];
  double brushBounds[6];
  double sRadius = (m_plane == SAGITTAL)?0:m_radius;
  double cRadius = (m_plane ==  CORONAL)?0:m_radius;
  double aRadius = (m_plane ==    AXIAL)?0:m_radius;

  brushBounds[0] = brush[0] - sRadius;
  brushBounds[1] = brush[0] + sRadius;
  brushBounds[2] = brush[1] - cRadius;
  brushBounds[3] = brush[1] + cRadius;
  brushBounds[4] = brush[2] - aRadius;
  brushBounds[5] = brush[2] + aRadius;

  BoundingBox previewBB(m_pBounds);
  BoundingBox brushBB(brushBounds);

  if (previewBB.intersect(brushBB))
  {
    BoundingBox updateBB = previewBB.intersection(brushBB);
    EspinaVolume::RegionType normRegion = BoundsToRegion(updateBB.bounds(), m_spacing);

    double r2 = m_radius*m_radius;
    VolumeIterator it(m_preview, normRegion);
    it.GoToBegin();
    for (; !it.IsAtEnd(); ++it )
    {
      double pixel[3];
      pixel[0] = it.GetIndex()[0]*m_spacing[0];
      pixel[1] = it.GetIndex()[1]*m_spacing[1];
      pixel[2] = it.GetIndex()[2]*m_spacing[2];
      if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
        it.Set(SEG_VOXEL_VALUE);
    }
    m_preview->Modified();
    view->updateView();
  }
}

//-----------------------------------------------------------------------------
void BrushPicker::stopPreview(EspinaRenderView* view)
{
  if (!m_previewVisible)
    return;

  view->removePreview(m_actor);
//   m_actor->Delete();
//   itk2vtk->Delete();
//   m_preview->Delete();
}
