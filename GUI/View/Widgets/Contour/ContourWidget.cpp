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
#include <GUI/View/Widgets/Contour/ContourWidget.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourWidget.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.h>
#include <GUI/View/View2D.h>
#include <Core/Utils/vtkPolyDataUtils.h>

// C++
#include <iostream>

//VTK
#include <vtkAbstractWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyData.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
ContourWidget::ContourWidget()
: m_tolerance  {40}
, m_color      {Qt::black}
{
}

//----------------------------------------------------------------------------
ContourWidget::~ContourWidget()
{
  for(auto view: m_widgets.keys())
  {
    unregisterView(view);
  }

  m_widgets.clear();
}

//----------------------------------------------------------------------------
void ContourWidget::registerView(RenderView *view)
{
  auto view2d = dynamic_cast<View2D *>(view);

  if (!view2d || m_widgets.keys().contains(view2d)) return;

  auto plane = view2d->plane();
  auto position = view2d->slicingPosition();

  connect(view, SIGNAL(crosshairPlaneChanged(Plane, Nm)), this, SLOT(changeSlice(Plane, Nm)));
  m_widgets[view2d] = vtkSmartPointer<vtkPlaneContourWidget>::New();
  m_widgets[view2d]->setContourWidget(this);
  m_widgets[view2d]->SetOrientation(plane);
  m_widgets[view2d]->setPolygonColor(m_color);
  m_widgets[view2d]->setActorsShift(view2d->widgetDepth());
  m_widgets[view2d]->setSlice(position);
  m_widgets[view2d]->SetContinuousDrawTolerance(m_tolerance);
  m_widgets[view2d]->SetCurrentRenderer(view->mainRenderer());
  m_widgets[view2d]->SetInteractor(view->renderWindow()->GetInteractor());
  m_widgets[view2d]->EnabledOn();

  m_storedContours[view2d] = ContourData(position, position, plane, DrawingMode::PAINTING, nullptr);
}

//----------------------------------------------------------------------------
void ContourWidget::unregisterView(RenderView *view)
{
  auto view2d = dynamic_cast<View2D *>(view);

  if (!m_widgets.keys().contains(view2d)) return;

  startContourFromWidget(); // to rasterize any pending contour in the view.

  disconnect(view, SIGNAL(crosshairPlaneChanged(Plane, Nm)), this, SLOT(changeSlice(Plane, Nm)));
  m_widgets[view2d]->EnabledOff();
  m_widgets[view2d]->SetInteractor(nullptr);
  m_widgets[view2d]->SetCurrentRenderer(nullptr);
  m_widgets[view2d] = nullptr;
  m_widgets.remove(view2d);

  m_storedContours[view2d].polyData = nullptr;
  m_storedContours.remove(view2d);
}

//----------------------------------------------------------------------------
void ContourWidget::setEnabled(bool enable)
{
  for(auto widget: m_widgets.values())
  {
    widget->SetEnabled(enable);
  }
}

//----------------------------------------------------------------------------
void ContourWidget::setPolygonColor(QColor color)
{
  if(m_color == color) return;

  m_color = color;

  for(auto widget: m_widgets.values())
  {
    widget->setPolygonColor(color);
  }
}

//----------------------------------------------------------------------------
QColor ContourWidget::polygonColor()
{
  return m_color;
}

//----------------------------------------------------------------------------
ContourWidget::ContourList ContourWidget::contours()
{
  ContourList resultList;

  for(auto view: m_widgets.keys())
  {
    auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(m_widgets[view]->GetRepresentation());
    auto polyData = rep->GetContourRepresentationAsPolyData();
    if (polyData)
    {
      auto contour = vtkSmartPointer<vtkPolyData>::New();
      contour->DeepCopy(polyData);
      polyData->Delete();

      auto plane = view->plane();
      auto position = view->slicingPosition();

      resultList << ContourData(position, position, plane, m_widgets[view]->contourMode(), contour);
    }
  }

  if(resultList.empty())
  {
    for(auto data: m_storedContours.values())
    {
      if(data.polyData)
      {
        resultList << ContourData(data.contourPosition, data.contourPosition, data.plane, data.mode, data.polyData);
      }
    }
  }

  return resultList;
}

//----------------------------------------------------------------------------
void ContourWidget::startContourFromWidget()
{
  ContourList resultList = contours();

  if(!resultList.empty())
  {
    for(auto data: resultList)
    {
      if(data.polyData)
      {
        auto mask = PolyDataUtils::rasterizeContourToMask(data.polyData, data.plane, data.contourPosition, m_spacing);
        auto value = data.mode == DrawingMode::PAINTING ? SEG_VOXEL_VALUE : SEG_BG_VALUE;
        mask->setForegroundValue(value);

        emit contour(mask);

        for(auto view: m_storedContours.keys())
        {
          m_storedContours[view].polyData = nullptr;
        }
      }
    }

    initialize();
  }
}

//----------------------------------------------------------------------------
void ContourWidget::setDrawingMode(DrawingMode mode)
{
  for(auto widget: m_widgets.values())
  {
    widget->setContourMode(mode);
  }
}

//----------------------------------------------------------------------------
void ContourWidget::initialize()
{
  for(auto widget: m_widgets.values())
  {
    widget->Initialize();
  }
}

//----------------------------------------------------------------------------
void ContourWidget::initialize(ContourData contour)
{
  if (contour.polyData == nullptr)
  {
    initialize();
  }

  for(auto view: m_widgets.keys())
  {
    if(view->plane() == contour.plane)
    {
      m_widgets[view]->setContourMode(contour.mode);
      m_widgets[view]->Initialize(contour.polyData);
    }
    else
    {
      m_widgets[view]->Initialize();
    }
  }
}

//----------------------------------------------------------------------------
void ContourWidget::changeSlice(Plane plane, Nm pos)
{
  View2D* view = dynamic_cast<View2D*>(sender());
  if(!m_storedContours.keys().contains(view) || m_storedContours[view].actualPosition == pos) return;

  auto widget = m_widgets[view];

  if(m_storedContours[view].polyData)
  {
    if(m_storedContours[view].contourPosition == pos)
    {
      widget->setSlice(pos);
      widget->setContourMode(m_storedContours[view].mode);
      widget->Initialize(m_storedContours[view].polyData.GetPointer());

      m_storedContours[view].polyData = nullptr;
    }
  }
  else
  {
    auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(widget->GetRepresentation());
    auto polyData = rep->GetContourRepresentationAsPolyData();
    if (polyData)
    {
      m_storedContours[view].polyData = vtkSmartPointer<vtkPolyData>::New();
      m_storedContours[view].polyData->DeepCopy(polyData);
      m_storedContours[view].contourPosition = m_storedContours[view].actualPosition;
      m_storedContours[view].mode = widget->contourMode();
      polyData->Delete();
    }

    widget->setSlice(pos);
    widget->Initialize();
  }

  m_storedContours[view].actualPosition = pos;
}

//----------------------------------------------------------------------------
void ContourWidget::setMinimumPointDistance(const Nm distance)
{
  m_tolerance = distance;

  for(auto widget: m_widgets.values())
  {
    widget->SetContinuousDrawTolerance(m_tolerance);
  }
}
