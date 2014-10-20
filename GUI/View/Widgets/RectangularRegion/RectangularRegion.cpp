/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "RectangularRegion.h"
#include "vtkRectangularSliceWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWidgetRepresentation.h>

// Qt
#include <QDebug>
#include <QMouseEvent>

using namespace ESPINA;

using SliceWidgetAdapter = EspinaInteractorAdapter<vtkRectangularSliceWidget>;

//----------------------------------------------------------------------------
RectangularRegion::RectangularRegion(Bounds bounds)
: m_bounds     {bounds}
, m_pattern    {0xFFFF}
, m_command    {vtkSmartPointer<vtkRectangularRegionCommand>::New()}
{
  m_command->setWidget(this);

  m_resolution[0] = m_resolution[1] = m_resolution[2] = 1;

  // default color
  m_color[0] = m_color[1] = 1.0;
  m_color[2] = 0.0;
}

//----------------------------------------------------------------------------
RectangularRegion::~RectangularRegion()
{
  for(auto view: m_widgets.keys())
    unregisterView(view);

  m_widgets.clear();
}

//----------------------------------------------------------------------------
void RectangularRegion::registerView(RenderView *view)
{
  View2D *view2d = dynamic_cast<View2D *>(view);
  if(view2d && !m_widgets.keys().contains(view))
  {
    SliceWidgetAdapter *wi = SliceWidgetAdapter::New();
    Q_ASSERT(wi);
    wi->AddObserver(vtkCommand::EndInteractionEvent, m_command);
    wi->SetView(view2d);
    wi->SetPlane(view2d->plane());
    wi->SetSlice(view2d->crosshairPoint()[normalCoordinateIndex(view2d->plane())]);
    wi->SetBounds(m_bounds);
    wi->setRepresentationColor(m_color);
    wi->setRepresentationPattern(m_pattern);
    wi->SetInteractor(view2d->renderWindow()->GetInteractor());
    wi->SetCurrentRenderer(view2d->mainRenderer());
    wi->SetEnabled(true);

    m_widgets[view] = wi;

    connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::unregisterView(RenderView *view)
{
  View2D *view2d = dynamic_cast<View2D *>(view);
  if(view2d && m_widgets.keys().contains(view))
  {
    disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));

    m_widgets[view]->EnabledOff();
    m_widgets[view]->RemoveObserver(m_command);
    m_widgets[view]->SetCurrentRenderer(nullptr);
    m_widgets[view]->SetInteractor(nullptr);
    m_widgets[view]->Delete();

    m_widgets.remove(view);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setEnabled(bool enable)
{
  for(auto widget: m_widgets.values())
  {
    widget->SetProcessEvents(enable);
    widget->GetRepresentation()->SetPickable(enable);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setBounds(Bounds bounds)
{
  for(auto widget: m_widgets.values())
  {
    widget->SetBounds(bounds);
  }
}

//----------------------------------------------------------------------------
Bounds RectangularRegion::bounds() const
{
  Q_ASSERT(!m_widgets.isEmpty());

  auto widget = m_widgets.begin().value();

  return widget->GetBounds();
}
//----------------------------------------------------------------------------
void RectangularRegion::setResolution(NmVector3 resolution)
{
  m_resolution = resolution;

//   for(int i = 0; i < 6; i++)
//   {
//     m_bounds[i] = int(m_bounds[i]/m_resolution[i/2])*m_resolution[i/2];
//   }

  for(auto w: m_widgets.values())
  {
    w->SetBounds(m_bounds);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setRepresentationColor(double *color)
{
  if (0 == memcmp(m_color, color, sizeof(double)*3))
    return;

  memcpy(m_color, color, sizeof(double)*3);

  for(auto widget: m_widgets.values())
    widget->setRepresentationColor(m_color);
}

//----------------------------------------------------------------------------
void RectangularRegion::setRepresentationPattern(int pattern)
{
  if (pattern == m_pattern)
    return;

  m_pattern = pattern;

  for(auto widget: m_widgets.values())
    widget->setRepresentationPattern(m_pattern);
}

//----------------------------------------------------------------------------
void RectangularRegion::sliceChanged(Plane plane, Nm pos)
{
  View2D *view2d = dynamic_cast<View2D *>(sender());
  RenderView *view = dynamic_cast<RenderView *>(sender());
  if(view2d && view && m_widgets.keys().contains(view))
    m_widgets[view]->SetSlice(pos);
}

//----------------------------------------------------------------------------
void vtkRectangularRegionCommand::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  vtkRectangularSliceWidget *widget = static_cast<vtkRectangularSliceWidget *>(caller);

  if (widget)
  {
    m_widget->m_bounds = widget->GetBounds();

    for(auto w: m_widget->m_widgets.values())
    {
      w->SetBounds(m_widget->m_bounds);
    }
  }

  m_widget->emitModifiedSignal();
}

//----------------------------------------------------------------------------
void vtkRectangularRegionCommand::setWidget(EspinaWidgetPtr widget)
{
  m_widget = dynamic_cast<RectangularRegion *>(widget);
}
