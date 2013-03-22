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

// EspINA
#include "GUI/QtWidget/EspinaRenderView.h"
#include "GUI/ViewManager.h"
#include <Core/Model/Channel.h>
#include <Filters/FreeFormSource.h>
#include <Core/EspinaRegion.h>
#include "GUI/QtWidget/SliceView.h"

// Qt
#include <QMouseEvent>
#include <QPainter>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkCoordinate.h>
#include <vtkSphere.h>
#include <vtkImageResliceToColors.h>
#include <vtkMatrix4x4.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
BrushPicker::BrushPicker(PickableItemPtr item)
: m_referenceItem(item)
, m_displayRadius(20)
, m_borderColor(Qt::blue)
, m_brushColor(Qt::blue)
, m_brushImage(NULL)
, m_tracking(false)
, m_stroke(IPicker::WorldRegion::New())
, m_plane(AXIAL)
, m_radius(-1)
, m_lut(NULL)
, m_preview(NULL)
, m_actor(NULL)
, m_drawing(true)
, m_segmentation(NULL)
{
  memset(m_viewSize, 0, 2*sizeof(int));
  memset(m_LL, 0, 3*sizeof(double));
  memset(m_UR, 0, 3*sizeof(double));
  memset(m_pBounds, 0, 6*sizeof(Nm));
  memset(m_worldSize, 0, 2*sizeof(double));
}

//-----------------------------------------------------------------------------
BrushPicker::~BrushPicker()
{
  if (m_brushImage)
  {
    delete m_brushImage;
    m_brushImage = NULL;
  }
}

//-----------------------------------------------------------------------------
bool BrushPicker::filterEvent(QEvent* e, EspinaRenderView* view)
{
  if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && m_tracking)
    {
      startStroke(m_lastDot, view);
      return true;
    }
  }

  if (!m_tracking && QEvent::MouseButtonPress == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent *>(e);
    if (me->button() == Qt::LeftButton)
    {
      m_tracking = true;
      startStroke(me->pos(), view);
      return true;
    }
  }
  else
    if (m_tracking && QEvent::MouseMove == e->type())
    {
      QMouseEvent *me = static_cast<QMouseEvent*>(e);
      updateStroke(me->pos(), view);
      return true;
    }
    else
      if (m_tracking && QEvent::MouseButtonRelease == e->type())
      {
        m_tracking = false;
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
void BrushPicker::setReferenceItem(PickableItemPtr item)
{
  m_referenceItem = item;
  m_spacing = m_referenceItem->volume()->toITK()->GetSpacing();
}

//-----------------------------------------------------------------------------
void BrushPicker::setBrushImage(QImage &image)
{
  if (m_brushImage)
  {
    delete m_brushImage;
    m_brushImage = NULL;
  }

  if (!image.isNull())
  {
    m_brushImage = new QImage(image);
  }

  buildCursor();
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

  if (m_brushImage)
    p.drawImage(QPoint(m_displayRadius/2,m_displayRadius/2), m_brushImage->scaledToWidth(m_displayRadius));

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
  wPos[H] = m_LL[H] + pos.x()*m_worldSize[0]/m_viewSize[0];
  wPos[V] = m_UR[V] + pos.y()*m_worldSize[1]/m_viewSize[1];

  for(int i=0; i < 3; i++)
    brush[i] = int(wPos[i]/m_spacing[i]+0.5)*m_spacing[i];
}


//-----------------------------------------------------------------------------
bool BrushPicker::validStroke(double brush[3])
{
  double brushBounds[6];

  switch(m_plane)
  {
    case AXIAL:
      brushBounds[0] = brush[0] - m_radius;
      brushBounds[1] = brush[0] + m_radius;
      brushBounds[2] = brush[1] - m_radius;
      brushBounds[3] = brush[1] + m_radius;
      brushBounds[4] = brushBounds[5] = m_pBounds[4];
      break;
    case CORONAL:
      brushBounds[0] = brush[0] - m_radius;
      brushBounds[1] = brush[0] + m_radius;
      brushBounds[2] = brushBounds[3] = m_pBounds[2];
      brushBounds[4] = brush[2] - m_radius;
      brushBounds[5] = brush[2] + m_radius;
      break;
    case SAGITTAL:
      brushBounds[0] = brushBounds[1] = m_pBounds[0];
      brushBounds[2] = brush[1] - m_radius;
      brushBounds[3] = brush[1] + m_radius;
      brushBounds[4] = brush[2] - m_radius;
      brushBounds[5] = brush[2] + m_radius;
      break;
    default:
      break;
  }

  EspinaRegion previewBB(m_pBounds);
  EspinaRegion brushBB(brushBounds);

  if (!m_drawing)
  {
    // TODO: pixel-perfect collision of brush-segmentation
    // this method only eliminates strokes outside the bounding box.
    Nm bounds[6];
    m_segmentation->volume()->bounds(bounds);
    EspinaRegion segBB(bounds);
    return segBB.intersect(brushBB);
  }

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

  memcpy(m_viewSize, view->renderWindow()->GetSize(), 2*sizeof(int));

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

  m_radius = m_displayRadius*m_worldSize[0]/m_viewSize[0];

  double brush[3];
  createBrush(brush, pos);

  startPreview(view);

  if (validStroke(brush))
  {
    m_lastDot = pos;
    m_stroke->InsertNextPoint(brush);
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
    updatePreview(brush, view);
  }
}

//-----------------------------------------------------------------------------
void BrushPicker::stopStroke(EspinaRenderView* view)
{
  if (m_stroke->GetNumberOfPoints() > 0)
    emit stroke(m_referenceItem, m_stroke, m_radius, m_plane);

  m_stroke->Reset();
  stopPreview(view);
}

//-----------------------------------------------------------------------------
void BrushPicker::startPreview(EspinaRenderView* view)
{
  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), 1.0);
  m_lut->Modified();

  int extent[6];
  for (int i = 0; i < 3; ++i)
  {
    extent[2 * i] = m_pBounds[2 * i] / m_spacing[i];
    extent[(2 * i) + 1] = m_pBounds[(2 * i) + 1] / m_spacing[i];
  }

  m_preview = vtkSmartPointer<vtkImageData>::New();
  m_preview->SetOrigin(0, 0, 0);
  m_preview->SetScalarTypeToUnsignedChar();
  m_preview->SetExtent(extent);
  m_preview->SetSpacing(m_spacing[0], m_spacing[1], m_spacing[2]);
  m_preview->AllocateScalars();
  memset(m_preview->GetScalarPointer(), 0, m_preview->GetNumberOfPoints());

  // if erasing hide seg and copy contents of slice to preview actor
  if (!m_drawing)
  {
    SegmentationList list;
    list.append(m_segmentation);
    reinterpret_cast<SliceView *>(view)->hideSegmentations(list);

    int segExtent[6];
    m_segmentation->volume()->extent(segExtent);

    // minimize voxel copy, only fill the part of the preview that has
    // segmentation voxels.
    if (m_plane != SAGITTAL)
    {
      segExtent[0] = (extent[0] > segExtent[0]) ? extent[0] : segExtent[0];
      segExtent[1] = (extent[1] < segExtent[1]) ? extent[1] : segExtent[1];
    }
    else
      segExtent[0] = segExtent[1] = extent[0];

    if (m_plane != CORONAL)
    {
      segExtent[2] = (extent[2] > segExtent[2]) ? extent[2] : segExtent[2];
      segExtent[3] = (extent[3] < segExtent[3]) ? extent[3] : segExtent[3];
    }
    else
      segExtent[2] = segExtent[3] = extent[2];

    if (m_plane != AXIAL)
    {
      segExtent[4] = (extent[4] > segExtent[4]) ? extent[4] : segExtent[4];
      segExtent[5] = (extent[5] < segExtent[5]) ? extent[5] : segExtent[5];
    }
    else
      segExtent[4] = segExtent[5] = extent[4];

    unsigned char *previewPixel;
    itkVolumeType::IndexType index;

    // segmentations loaded from disk can have an origin != (0,0,0) that messes with the preview
    // and must be corrected. That means 3 ops more per loop and 3 more vars. If corrected before
    // we could use the index as the "for" variables directly, so it would be faster.
    itkVolumeType::PointType origin = m_segmentation->volume()->toITK()->GetOrigin();

    for (int x = segExtent[0]; x <= segExtent[1]; ++x)
      for (int y = segExtent[2]; y <= segExtent[3]; ++y)
        for (int z = segExtent[4]; z <= segExtent[5]; ++z)
        {
          index[0] = x - origin[0];
          index[1] = y - origin[1];
          index[2] = z - origin[2];

          previewPixel = reinterpret_cast<unsigned char *>(m_preview->GetScalarPointer(x,y,z));
          *previewPixel = m_segmentation->volume()->toITK()->GetPixel(index);
        }
  }

  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  if (m_pBounds[4] == m_pBounds[5])
  {
    double elements[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, m_pBounds[4], 0, 0, 0, 1 };
    matrix->DeepCopy(elements);
    m_preview->SetOrigin(0, 0, m_pBounds[4] - (extent[4] * m_spacing[2])); // resolves "Fit to slices" discrepancy
  }
  else
    if (m_pBounds[2] == m_pBounds[3])
    {
      double elements[16] = { 1, 0, 0, 0, 0, 0, 1, m_pBounds[2], 0, -1, 0, 0, 0, 0, 0, 1 };
      matrix->DeepCopy(elements);
      m_preview->SetOrigin(0, m_pBounds[2] - (extent[2] * m_spacing[1]), 0); // resolves "Fit to slices" discrepancy
    }
    else
      if (m_pBounds[0] == m_pBounds[1])
      {
        double elements[16] = { 0, 0, -1, m_pBounds[0], 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
        matrix->DeepCopy(elements);
        m_preview->SetOrigin(m_pBounds[0] - (extent[0] * m_spacing[0]), 0, 0); // resolves "Fit to slices" discrepancy
      }

  m_preview->Update();

  vtkSmartPointer<vtkImageResliceToColors> reslice = vtkSmartPointer<vtkImageResliceToColors>::New();
  reslice->OptimizationOn();
  reslice->BorderOn();
  reslice->SetOutputFormatToRGBA();
  reslice->AutoCropOutputOff();
  reslice->InterpolateOff();
  reslice->SetResliceAxes(matrix);
  reslice->SetInputConnection(m_preview->GetProducerPort());
  reslice->SetOutputDimensionality(2);
  reslice->SetLookupTable(m_lut);
  reslice->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetPickable(false);
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
  m_actor->Update();

  // reposition actor to be over plane
  double pos[3];
  memset(pos, 0, 3 * sizeof(double));
  int sign = ((m_plane == AXIAL) ? -1 : 1);
  pos[m_plane] += (sign * 0.1);
  m_actor->SetPosition(pos);

  view->addActor(m_actor);
  view->updateView();
}

//-----------------------------------------------------------------------------
void BrushPicker::updatePreview(double brush[3], EspinaRenderView* view)
{
  // fixes crashes when the user releases or presses the control key
  // in the middle of a stroke (that is, without unpressing the mouse
  // button).
  if (!m_preview)
    startPreview(view);

  double brushBounds[6];

  switch(m_plane)
  {
    case AXIAL:
      brushBounds[0] = brush[0] - m_radius;
      brushBounds[1] = brush[0] + m_radius;
      brushBounds[2] = brush[1] - m_radius;
      brushBounds[3] = brush[1] + m_radius;
      brushBounds[4] = brushBounds[5] = m_pBounds[4];
      break;
    case CORONAL:
      brushBounds[0] = brush[0] - m_radius;
      brushBounds[1] = brush[0] + m_radius;
      brushBounds[2] = brushBounds[3] = m_pBounds[2];
      brushBounds[4] = brush[2] - m_radius;
      brushBounds[5] = brush[2] + m_radius;
      break;
    case SAGITTAL:
      brushBounds[0] = brushBounds[1] = m_pBounds[0];
      brushBounds[2] = brush[1] - m_radius;
      brushBounds[3] = brush[1] + m_radius;
      brushBounds[4] = brush[2] - m_radius;
      brushBounds[5] = brush[2] + m_radius;
      break;
    default:
      break;
  }

  EspinaRegion previewBB(m_pBounds);
  EspinaRegion brushBB(brushBounds);

  if (previewBB.intersect(brushBB))
  {
    EspinaRegion updateBB = previewBB.intersection(brushBB);
    const Nm * bounds = updateBB.bounds();

    double r2 = m_radius*m_radius;
    switch(m_plane)
    {
      case AXIAL:
        for (int x = bounds[0]/m_spacing[0]; x <= bounds[1]/m_spacing[0]; x++)
          for (int y = bounds[2]/m_spacing[1]; y <= bounds[3]/m_spacing[1]; y++)
        {
          double pixel[3] = {x*m_spacing[0], y*m_spacing[1], m_pBounds[4]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,y,m_pBounds[4]/m_spacing[2]));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      case CORONAL:
        for (int x = bounds[0]/m_spacing[0]; x <= bounds[1]/m_spacing[0]; x++)
          for (int z = bounds[4]/m_spacing[2]; z <= bounds[5]/m_spacing[2]; z++)
        {
          double pixel[3] = {x*m_spacing[0], m_pBounds[2], z*m_spacing[2]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,m_pBounds[2]/m_spacing[1],z));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      case SAGITTAL:
        for (int y = bounds[2]/m_spacing[1]; y <= bounds[3]/m_spacing[1]; y++)
          for (int z = bounds[4]/m_spacing[2]; z <= bounds[5]/m_spacing[2]; z++)
        {
          double pixel[3] = {m_pBounds[0], y*m_spacing[1], z*m_spacing[2]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(m_pBounds[0]/m_spacing[0],y,z));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      default:
        break;
    }
    m_preview->Modified();
    view->updateView();
  }
}

//-----------------------------------------------------------------------------
void BrushPicker::stopPreview(EspinaRenderView* view)
{
  view->removeActor(m_actor);
  m_lut = NULL;
  m_preview = NULL;
  m_actor = NULL;

  if (!m_drawing && m_segmentation != NULL)
  {
    SegmentationList list;
    list.append(m_segmentation);
    reinterpret_cast<SliceView *>(view)->unhideSegmentations(list);
  }
  view->updateView();
}

//-----------------------------------------------------------------------------
void BrushPicker::DrawingOn(EspinaRenderView *view)
{
  if ((m_stroke->GetNumberOfPoints() > 0) && view != NULL)
    stopStroke(view);

  m_drawing = true;
  m_segmentation = NULL;
}

//-----------------------------------------------------------------------------
void BrushPicker::DrawingOff(EspinaRenderView *view, SegmentationPtr segmentation)
{
  if (m_stroke->GetNumberOfPoints() > 0)
    stopStroke(view);

  m_drawing = false;
  m_segmentation = segmentation;
}

//-----------------------------------------------------------------------------
QColor BrushPicker::getBrushColor()
{
  return m_brushColor;
}
