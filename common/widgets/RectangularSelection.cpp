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


#include "RectangularSelection.h"

#include "vtkNonRotatingBoxWidget.h"
#include "vtkRectangularSliceWidget.h"
#include <EspinaCore.h>
#include <EspinaView.h>
#include <vtkWidgetRepresentation.h>


#include <QDebug>

class RectangularSliceWidget
: public SliceWidget
{
public:
    explicit RectangularSliceWidget(vtkRectangularSliceWidget* widget)
    : SliceWidget(widget)
    , m_widget(widget)
    { }
    virtual void setSlice(Nm pos, PlaneType plane)
    { m_widget->SetSlice(pos); }

private:
  vtkRectangularSliceWidget *m_widget;
};

//----------------------------------------------------------------------------
RectangularRegion::RectangularRegion(double bounds[6])
{
  memcpy(m_bounds, bounds, 6*sizeof(double));
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
vtkAbstractWidget* RectangularRegion::createWidget()
{
  return NULL;
}
//----------------------------------------------------------------------------
void RectangularRegion::deleteWidget(vtkAbstractWidget* widget)
{
  Q_ASSERT(false);
}


//----------------------------------------------------------------------------
SliceWidget* RectangularRegion::createSliceWidget(PlaneType plane)
{
  vtkRectangularSliceWidget *w = vtkRectangularSliceWidget::New();
  Q_ASSERT(w);
  w->AddObserver(vtkCommand::EndInteractionEvent, this);
  w->SetPlane(plane);
  w->SetBounds(m_bounds);

  m_widgets << w;

  return new RectangularSliceWidget(w);
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
void RectangularRegion::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  vtkRectangularSliceWidget *widget = static_cast<vtkRectangularSliceWidget *>(caller);

  if (widget)
  {
    widget->GetBounds(m_bounds);

    foreach(vtkRectangularSliceWidget *w, m_widgets)
      if (w != widget)
        w->SetBounds(m_bounds);
  }
  emit modified(m_bounds);
  EspinaCore::instance()->viewManger()->currentView()->forceRender();
}
