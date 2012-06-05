/*=========================================================================

   Program: ParaView
   Module:    pqNonRotatingBoxWidget.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqNonRotatingBoxWidget.h"

// Server Manager Includes.
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>

// Qt Includes.
#include <QDoubleValidator>

// ParaView Includes.
#include <pq3DWidgetFactory.h>
#include <pqApplicationCore.h>
#include <pqPropertyLinks.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <pqSMAdaptor.h>
#include "vtkNonRotatingBoxRepresentation.h"

//-----------------------------------------------------------------------------
pqNonRotatingBoxWidget::pqNonRotatingBoxWidget(vtkSMProxy* refProxy, vtkSMProxy* pxy, QWidget* _parent) :
  Superclass(refProxy, pxy, _parent)
{

  QObject::connect(this, SIGNAL(widgetVisibilityChanged(bool)),
    this, SLOT(onWidgetVisibilityChanged(bool)));

  //QObject::connect(this, SIGNAL(widgetStartInteraction()),
  //  this, SLOT(showHandles()));

  pqServerManagerModel* smmodel =
    pqApplicationCore::instance()->getServerManagerModel();
  this->createWidget(smmodel->findServer(refProxy->GetSession()));
}

//-----------------------------------------------------------------------------
pqNonRotatingBoxWidget::~pqNonRotatingBoxWidget()
{
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::createWidget(pqServer* server)
{
  vtkSMNewWidgetRepresentationProxy* widget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("NonRotatingBoxWidgetRepresentation", server);
  this->setWidgetProxy(widget);

  widget->UpdateVTKObjects();
  widget->UpdatePropertyInformation();
}

//-----------------------------------------------------------------------------
// update widget bounds.
void pqNonRotatingBoxWidget::select()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  double input_bounds[6];
  if (widget  && this->getReferenceInputBounds(input_bounds))
    {
    vtkSMPropertyHelper(widget, "PlaceWidget").Set(input_bounds, 6);
    widget->UpdateVTKObjects();
    }

  this->Superclass::select();
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::resetBounds(double input_bounds[6])
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  vtkSMPropertyHelper(widget, "PlaceWidget").Set(input_bounds, 6);
  widget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::cleanupWidget()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if(widget)
    {
    pqApplicationCore::instance()->get3DWidgetFactory()->
      free3DWidget(widget);
    }
  this->setWidgetProxy(0);
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::onWidgetVisibilityChanged(bool visible)
{
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::accept()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  Q_ASSERT(widget);
  vtkSMProxy *proxy = widget->GetRepresentationProxy();
  Q_ASSERT(proxy);
  vtkNonRotatingBoxRepresentation *rep =
    dynamic_cast<vtkNonRotatingBoxRepresentation *>(proxy->GetClientSideObject());
  Q_ASSERT(rep);
  vtkSMPropertyHelper(getControlledProxy(), "Bounds").Set(rep->GetBounds(), 6);
  getControlledProxy()->UpdateVTKObjects();
  this->Superclass::accept();
  this->hideHandles();
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::reset()
{
  this->Superclass::reset();
  this->hideHandles();
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::showHandles()
{
  /*
  vtkSMProxy* proxy = this->getWidgetProxy();
  if (proxy)
    {
    pqSMAdaptor::setElementProperty(proxy->GetProperty("HandleVisibility"), 1);
    proxy->UpdateVTKObjects();
    }
    */
}

//-----------------------------------------------------------------------------
void pqNonRotatingBoxWidget::hideHandles()
{
  /*
  vtkSMProxy* proxy = this->getWidgetProxy();
  if (proxy)
    {
    pqSMAdaptor::setElementProperty(proxy->GetProperty("HandleVisibility"), 1);
    proxy->UpdateVTKObjects();
    }
  */
}
