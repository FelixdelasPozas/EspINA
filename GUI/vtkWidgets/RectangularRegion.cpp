/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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


#include "RectangularRegion.h"

#include "EspinaInteractorAdapter.h"
#include "vtkNonRotatingBoxWidget.h"
#include "vtkRectangularSliceWidget.h"

#include "GUI/QtWidget/EspinaRenderView.h"
#include <GUI/QtWidget/SliceView.h>
#include "GUI/ViewManager.h"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWidgetRepresentation.h>

#include <QDebug>
#include <QMouseEvent>

using namespace EspINA;

typedef EspinaInteractorAdapter<vtkRectangularSliceWidget> SliceWidgetAdapter;

class RectangularSliceWidget
: public SliceWidget
{
public:
    explicit RectangularSliceWidget(vtkRectangularSliceWidget *widget)
    : SliceWidget(widget)
    , m_widget(widget)
    { }
    virtual void setSlice(Nm pos, PlaneType plane)
    { m_widget->SetSlice(pos); }

private:
  vtkRectangularSliceWidget *m_widget;
};

//----------------------------------------------------------------------------
RectangularRegion::RectangularRegion(double bounds[6], ViewManager *vm)
: m_viewManager(vm)
, m_pattern(0xFFFF)
{
  memcpy(m_bounds, bounds, 6*sizeof(double));
  m_resolution[0] = m_resolution[1] = m_resolution[2] = 1;

  // default color
  m_color[0] = m_color[1] = 1.0;
  m_color[2] = 0.0;
}

//----------------------------------------------------------------------------
RectangularRegion::~RectangularRegion()
{
  foreach(vtkAbstractWidget *w, m_widgets)
  {
    w->EnabledOff();
    w->RemoveAllObservers();
    w->Delete();
  }
  m_widgets.clear();
}

//----------------------------------------------------------------------------
vtkAbstractWidget* RectangularRegion::create3DWidget(VolumeView *view)
{
  return NULL;
}

//----------------------------------------------------------------------------
SliceWidget* RectangularRegion::createSliceWidget(SliceView *view)
{
  SliceWidgetAdapter *wi = new SliceWidgetAdapter();
  Q_ASSERT(wi);
  wi->AddObserver(vtkCommand::EndInteractionEvent, this);
  wi->SetPlane(view->plane());
  wi->SetBounds(m_bounds);
  wi->setRepresentationColor(m_color);
  wi->setRepresentationPattern(m_pattern);
  m_widgets << wi;

  return new RectangularSliceWidget(wi);
}

//----------------------------------------------------------------------------
bool RectangularRegion::processEvent(vtkRenderWindowInteractor* iren,
                                     long unsigned int event)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    if (widget->GetInteractor() == iren)
    {
      SliceWidgetAdapter *sw = dynamic_cast<SliceWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }
  }

  return false;
}

//----------------------------------------------------------------------------
void RectangularRegion::setEnabled(bool enable)
{
  vtkAbstractWidget *widget;
  foreach(widget, m_widgets)
  {
    widget->SetProcessEvents(enable);
    widget->GetRepresentation()->SetPickable(enable);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::setBounds(Nm bounds[6])
{
  foreach(vtkRectangularSliceWidget *widget, m_widgets)
  {
    widget->SetBounds(bounds);
  }
}

//----------------------------------------------------------------------------
void RectangularRegion::bounds(Nm bounds[6])
{
  Q_ASSERT(!m_widgets.isEmpty());

  vtkRectangularSliceWidget *widget = m_widgets[0];

  widget->GetBounds(bounds);
}
//----------------------------------------------------------------------------
void RectangularRegion::setResolution(Nm resolution[3])
{
  memcpy(m_resolution, resolution, 3*sizeof(Nm));
  for(int i = 0; i < 6; i++)
    m_bounds[i] = int(m_bounds[i]/m_resolution[i/2])*m_resolution[i/2];

  foreach(vtkRectangularSliceWidget *w, m_widgets)
    w->SetBounds(m_bounds);
}

//----------------------------------------------------------------------------
void RectangularRegion::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  vtkRectangularSliceWidget *widget = static_cast<vtkRectangularSliceWidget *>(caller);

  if (widget)
  {
    widget->GetBounds(m_bounds);
    for(int i = 0; i < 6; i++)
    {
      m_bounds[i] = int(m_bounds[i]/m_resolution[i/2])*m_resolution[i/2];
    }

    foreach(vtkRectangularSliceWidget *w, m_widgets)
      w->SetBounds(m_bounds);
  }
  emit modified(m_bounds);
  if (m_viewManager)
    m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
void RectangularRegion::setRepresentationColor(double *color)
{
  if (0 == memcmp(m_color, color, sizeof(double)*3))
    return;

  memcpy(m_color, color, sizeof(double)*3);

  foreach(vtkRectangularSliceWidget *widget, m_widgets)
    widget->setRepresentationColor(m_color);
}

//----------------------------------------------------------------------------
void RectangularRegion::setRepresentationPattern(int pattern)
{
  if (pattern == m_pattern)
    return;

  m_pattern = pattern;

  foreach(vtkRectangularSliceWidget *widget, m_widgets)
    widget->setRepresentationPattern(m_pattern);
}
