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

// EspINA
#include "TubularWidget.h"
#include "TubularSliceWidget.h"
#include <GUI/vtkWidgets/EspinaInteractorAdapter.h>

// Qt
#include <QDebug>

// VTK
#include <vtkWidgetRepresentation.h>

namespace EspINA
{
  typedef EspinaInteractorAdapter<vtkTubularWidget> TubularWidgetAdapter;

  //----------------------------------------------------------------------------
  TubularWidget::TubularWidget()
      : m_box(NULL)
  {
  }

  //----------------------------------------------------------------------------
  TubularWidget::~TubularWidget()
  {
    QList<vtkAbstractWidget*>::Iterator it;
    for (it = m_widgets.begin(); it != m_widgets.end(); it++)
    {
      (*it)->EnabledOn();
      (*it)->Delete();
    }

    m_widgets.clear();
  }

  //----------------------------------------------------------------------------
  vtkAbstractWidget* TubularWidget::createWidget()
  {
    return NULL;
  }

  //----------------------------------------------------------------------------
  void TubularWidget::deleteWidget(vtkAbstractWidget* widget)
  {
    Q_ASSERT(false);
  }

  //----------------------------------------------------------------------------
  SliceWidget* TubularWidget::createSliceWidget(PlaneType plane)
  {
    TubularWidgetAdapter *widget = new TubularWidgetAdapter();
    Q_ASSERT(widget);
    widget->AddObserver(vtkCommand::EndInteractionEvent, this);

    widget->SetPlane(plane);
    widget->SetNodeList(m_nodes);

    m_widgets << widget;
    m_sliceWidgets << widget;

    return new TubularSliceWidget(widget);
  }

  //----------------------------------------------------------------------------
  void TubularWidget::setEnabled(bool enable)
  {
    QList<vtkAbstractWidget*>::Iterator it;
    for (it = m_widgets.begin(); it != m_widgets.end(); it++)
    {
      (*it)->SetProcessEvents(enable);
      (*it)->GetRepresentation()->SetPickable(enable);
    }
  }

  //----------------------------------------------------------------------------
  void TubularWidget::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
  {
    vtkTubularWidget *widget = static_cast<vtkTubularWidget *>(caller);

    if (widget)
    {
      QList<vtkTubularWidget*>::Iterator it;
      for (it = m_sliceWidgets.begin(); it != m_sliceWidgets.end(); it++)
      {
        if ((*it) == widget)
          continue;

        (*it)->SetNodeList(widget->GetNodeList());
      }
      emit nodesUpdated(widget->GetNodeList());
    }
  }

  //----------------------------------------------------------------------------
  void TubularWidget::setNodes(TubularSegmentationFilter::NodeList nodes)
  {
    if (m_nodes != nodes)
    {
      m_nodes = nodes;
      foreach(vtkTubularWidget *w, m_sliceWidgets)w->SetNodeList(m_nodes);

      emit
      nodesUpdated(m_nodes);
    }
  }

  //----------------------------------------------------------------------------
  void TubularWidget::setRoundExtremes(bool value)
  {
    foreach(vtkTubularWidget *w, m_sliceWidgets)if (value)
    w->RoundedExtremesOn();
    else
    w->RoundedExtremesOff();
  }

  //----------------------------------------------------------------------------
  bool TubularWidget::processEvent(vtkRenderWindowInteractor* iren, long unsigned int event)
  {
    foreach(vtkAbstractWidget *widget, m_widgets){
    if (widget->GetInteractor() == iren)
    {
      TubularWidgetAdapter *wa = dynamic_cast<TubularWidgetAdapter *>(widget);
      return wa->ProcessEventsHandler(event);
    }
  }

  return false;
  }

}
