/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

// EspINA
#include "RulerWidget.h"
#include "RulerSliceWidget.h"
#include "vtkRulerWidget.h"
#include "vtkRulerWidget3D.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkCubeAxesActor2D.h>

namespace EspINA
{
  using RulerWidgetAdapter = EspinaInteractorAdapter<vtkRulerWidget>;

  //----------------------------------------------------------------------------
  RulerWidget::RulerWidget()
  : m_axial(nullptr)
  , m_coronal(nullptr)
  , m_sagittal(nullptr)
  , m_volume(nullptr)
  {
  }
  
  //----------------------------------------------------------------------------
  RulerWidget::~RulerWidget()
  {
    for(auto widget: m_rulerSliceWidgets)
      delete widget;

    if (m_axial)
      m_axial->Delete();

    if (m_coronal)
      m_coronal->Delete();

    if (m_sagittal)
      m_sagittal->Delete();

    if (m_volume)
      m_volume->Delete();
  }

  //----------------------------------------------------------------------------
  vtkAbstractWidget *RulerWidget::create3DWidget(View3D *view)
  {
    m_volume = vtkRulerWidget3D::New();
    return m_volume;
  }

  //----------------------------------------------------------------------------
  SliceWidget *RulerWidget::createSliceWidget(View2D *view)
  {
    RulerSliceWidget *widget = nullptr;

    switch(view->plane())
    {
      case Plane::XY:
        m_axial = RulerWidgetAdapter::New();
        m_axial->setPlane(Plane::XY);
        widget = new RulerSliceWidget(m_axial);
        break;
      case Plane::XZ:
        m_coronal = RulerWidgetAdapter::New();
        m_coronal->setPlane(Plane::XZ);
        widget = new RulerSliceWidget(m_coronal);
        break;
      case Plane::YZ:
        m_sagittal = RulerWidgetAdapter::New();
        m_sagittal->setPlane(Plane::YZ);
        widget = new RulerSliceWidget(m_sagittal);
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    m_rulerSliceWidgets << widget;
    return widget;
  }

  //----------------------------------------------------------------------------
  bool RulerWidget::processEvent(vtkRenderWindowInteractor *iren,
                                   long unsigned int event)
  {
    if (m_axial)
    {
      QList<vtkRulerWidget *> list;
      list << m_axial << m_coronal << m_sagittal;
      for(auto widget: list)
        if (widget->GetInteractor() == iren)
        {
          RulerWidgetAdapter *sw = static_cast<RulerWidgetAdapter *>(widget);
          return sw->ProcessEventsHandler(event);
        }
    }

    return false;
  }

  //----------------------------------------------------------------------------
  void RulerWidget::setEnabled(bool enable)
  {
    if (!m_axial)
      return;

    m_axial->SetEnabled(enable);
    m_coronal->SetEnabled(enable);
    m_sagittal->SetEnabled(enable);
    m_volume->SetEnabled(enable);
  }

  //----------------------------------------------------------------------------
  void RulerWidget::setBounds(Bounds bounds)
  {
    if (!m_axial)
      return;

    m_axial->setBounds(bounds);
    m_coronal->setBounds(bounds);
    m_sagittal->setBounds(bounds);
    m_volume->setBounds(bounds);
    for(auto widget: m_rulerSliceWidgets)
    {
      widget->setBounds(bounds);
    }
  }

} /* namespace EspINA */
