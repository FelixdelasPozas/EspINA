/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/Utils/vtkPolyDataUtils.h>
#include <GUI/EventHandlers/ContourPainter.h>
#include <GUI/View/Widgets/Contour/ContourWidget2D.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourWidget.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Contour;

//----------------------------------------------------------------------------
ContourWidget2D::ContourWidget2D(ContourPainterSPtr handler)
: m_handler{handler}
, m_widget {vtkSmartPointer<vtkPlaneContourWidget>::New()}
, m_slice  {0}
, m_index  {0}
{
  m_widget->setParentWidget(this);

  connect(m_handler.get(), SIGNAL(clear()),
          this,            SLOT(initialize()));

  connect(m_handler.get(), SIGNAL(rasterize()),
          this,            SLOT(rasterize()));

  connect(m_handler.get(), SIGNAL(drawingModeChanged(DrawingMode)),
          this,            SLOT(setDrawingMode(DrawingMode)));

  connect(m_handler.get(), SIGNAL(configure(Nm, QColor, NmVector3)),
          this,            SLOT(configure(Nm, QColor, NmVector3)));

  connect(this,            SIGNAL(contour(BinaryMaskSPtr<unsigned char>)),
          m_handler.get(), SIGNAL(stopPainting(BinaryMaskSPtr<unsigned char>)));
}

//----------------------------------------------------------------------------
ContourWidget2D::~ContourWidget2D()
{
}

//----------------------------------------------------------------------------
void ContourWidget2D::setPlane(Plane plane)
{
  m_widget->SetOrientation(plane);

  m_index = normalCoordinateIndex(plane);
}

//----------------------------------------------------------------------------
void ContourWidget2D::setRepresentationDepth(Nm depth)
{
  m_widget->setActorsShift(depth);
}

//----------------------------------------------------------------------------
TemporalRepresentation2DSPtr ContourWidget2D::clone()
{
  return std::make_shared<ContourWidget2D>(m_handler);
}

//----------------------------------------------------------------------------
void ContourWidget2D::initialize(Contour contour)
{
  if(normalCoordinateIndex(contour.plane) != m_index) return;

  if(contour.slice == m_slice)
  {
    m_widget->Initialize(contour.polyData);
    m_widget->setContourMode(contour.mode);
    m_storedContour.polyData = nullptr;
  }
  else
  {
    m_storedContour = contour;
  }
}

//----------------------------------------------------------------------------
void ContourWidget2D::initialize()
{
  m_widget->Initialize();
  m_storedContour.polyData = nullptr;
}

//----------------------------------------------------------------------------
void ContourWidget2D::configure(Nm distance, QColor color, NmVector3 spacing)
{
  m_widget->SetContinuousDrawTolerance(distance);
  m_widget->setPolygonColor(color);
  m_spacing = spacing;
}

//----------------------------------------------------------------------------
void ContourWidget2D::setDrawingMode(DrawingMode mode)
{
  m_widget->setContourMode(mode);
}

//----------------------------------------------------------------------------
bool ContourWidget2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return true;
}

//----------------------------------------------------------------------------
bool ContourWidget2D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return true;
}

//----------------------------------------------------------------------------
void ContourWidget2D::initializeImplementation(RenderView* view)
{
}

//----------------------------------------------------------------------------
void ContourWidget2D::uninitializeImplementation()
{
}

//----------------------------------------------------------------------------
vtkAbstractWidget* ContourWidget2D::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void ContourWidget2D::setCrosshair(const NmVector3& crosshair)
{
  auto slice = crosshair[m_index];

  if (slice == m_slice) return;

  auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(m_widget->GetRepresentation());
  auto polyData = rep->GetContourRepresentationAsPolyData();
  if (polyData)
  {
    Q_ASSERT(!m_storedContour.polyData);
    auto contour = vtkSmartPointer<vtkPolyData>::New();
    contour->DeepCopy(polyData);
    polyData->Delete();

    m_storedContour.slice    = m_slice;
    m_storedContour.mode     = m_widget->contourMode();
    m_storedContour.plane    = toPlane(m_index);
    m_storedContour.polyData = contour;

    m_widget->Initialize();
  }

  m_slice = slice;
  m_widget->setSlice(m_slice);

  if (m_storedContour.polyData != nullptr && m_storedContour.slice == slice)
  {
    initialize(m_storedContour);
    Q_ASSERT(!m_storedContour.polyData);
  }

}

//----------------------------------------------------------------------------
void ContourWidget2D::rasterize()
{
  Contour contourToRasterize;

  auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(m_widget->GetRepresentation());
  auto polyData = rep->GetContourRepresentationAsPolyData();
  if (polyData)
  {
    auto newContour = vtkSmartPointer<vtkPolyData>::New();
    newContour->DeepCopy(polyData);
    polyData->Delete();

    contourToRasterize.polyData = newContour;
    contourToRasterize.plane    = toPlane(m_index);
    contourToRasterize.mode     = m_widget->contourMode();
    contourToRasterize.slice    = m_slice;
  }
  else
  {
    if(m_storedContour.polyData)
    {
      contourToRasterize = m_storedContour;
      m_storedContour.polyData = nullptr;
    }
  }

  if(contourToRasterize.polyData)
  {
    m_widget->Initialize();

    auto mask  = PolyDataUtils::rasterizeContourToMask(contourToRasterize.polyData, contourToRasterize.plane, contourToRasterize.slice, m_spacing);
    auto value = contourToRasterize.mode == DrawingMode::PAINTING ? SEG_VOXEL_VALUE : SEG_BG_VALUE;
    mask->setForegroundValue(value);

    emit contour(mask);
  }
}
