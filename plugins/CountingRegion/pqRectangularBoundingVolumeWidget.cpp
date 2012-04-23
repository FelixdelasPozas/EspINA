/*=========================================================================

   Program: ParaView
   Module:    pqRectangularBoundingVolumeWidget.cxx

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
#include "pqRectangularBoundingVolumeWidget.h"

// Server Manager Includes.
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"

// Qt Includes.
#include <QDoubleValidator>

// ParaView Includes.
#include "pq3DWidgetFactory.h"
#include "pqApplicationCore.h"
#include "pqPropertyLinks.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqSMAdaptor.h"
#include "vtkRectangularBoundingVolumeWidget.h"
#include <vtkPolyDataAlgorithm.h>

//-----------------------------------------------------------------------------
pqRectangularBoundingVolumeWidget::pqRectangularBoundingVolumeWidget(vtkSMProxy* refProxy, vtkSMProxy* pxy, QWidget* _parent) :
  Superclass(refProxy, pxy, _parent)
{
  QObject::connect(this, SIGNAL(widgetVisibilityChanged(bool)),
    this, SLOT(onWidgetVisibilityChanged(bool)));

  pqServerManagerModel* smmodel =
    pqApplicationCore::instance()->getServerManagerModel();
  this->createWidget(smmodel->findServer(refProxy->GetSession()));
}

//-----------------------------------------------------------------------------
pqRectangularBoundingVolumeWidget::~pqRectangularBoundingVolumeWidget()
{
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::createWidget(pqServer* server)
{
  vtkSMNewWidgetRepresentationProxy* widget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("RectangularBoundingVolumeWidgetRepresentation", server);
  this->setWidgetProxy(widget);

//   std::cout << "Create Widget" << std::endl;
  getControlledProxy()->UpdateSelfAndAllInputs();
  vtkPolyDataAlgorithm *region = vtkPolyDataAlgorithm::SafeDownCast(getControlledProxy()->GetClientSideObject());

  widget->UpdateVTKObjects();
  widget->UpdatePropertyInformation();

  if (region)
  {
//     std::cout << "Set Volume" << std::endl;
    vtkAbstractWidget *miwidget = widget->GetWidget();
    vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget)->SetVolume(region);
  }

}

//-----------------------------------------------------------------------------
// update widget bounds.
void pqRectangularBoundingVolumeWidget::select()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  double input_bounds[6];
  if (widget  && this->getReferenceInputBounds(input_bounds))
  {
    vtkSMPropertyHelper(widget, "PlaceWidget").Set(input_bounds, 6);
    widget->UpdateVTKObjects();

    vtkPolyDataAlgorithm *region = vtkPolyDataAlgorithm::SafeDownCast(getControlledProxy()->GetClientSideObject());
    if (region)
    {
      vtkAbstractWidget *miwidget = widget->GetWidget();
      vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget)->SetVolume(region);
    }
    //     updateWidgetMargins();
  }
  
  this->Superclass::select();
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::resetBounds(double input_bounds[6])
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  vtkSMPropertyHelper(widget, "PlaceWidget").Set(input_bounds, 6);
  widget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::cleanupWidget()
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
void pqRectangularBoundingVolumeWidget::updateControlledFilter()
{
//   std::cout << "Updating Controlled Filter" << std::endl;
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  vtkAbstractWidget *miwidget = widget->GetWidget();
  vtkRectangularBoundingVolumeWidget *rrbw = vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget);
  double offset[3];
  rrbw->GetInclusionOffset(offset);
//   std::cout << "Widget Inclusion Offset: " << offset[0] << " " << offset[1]  << " " << offset[2] << std::endl;
  vtkSMPropertyHelper(getControlledProxy(), "InclusionOffset").Set(offset, 3);
  rrbw->GetExclusionOffset(offset);
  vtkSMPropertyHelper(getControlledProxy(), "ExclusionOffset").Set(offset, 3);
//   std::cout << "Widget Exclusion Offset: " << offset[0] << " " << offset[1]  << " " << offset[2] << std::endl;
  getControlledProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::updateWidgetMargins()
{
//   std::cout << "Update Widget Margins" << std::endl;
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  vtkAbstractWidget *miwidget = widget->GetWidget();
  vtkRectangularBoundingVolumeWidget *rrbw = vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget);
  double offset[3];
  getControlledProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(getControlledProxy(), "InclusionOffset").Get(offset, 3);
  vtkSMPropertyHelper(widget, "InclusionOffset").Set(offset, 3);
//   std::cout << "Proxy Inclusion Offset: " << offset[0] << " " << offset[1]  << " " << offset[2] << std::endl;
  rrbw->SetInclusionOffset(offset);
  vtkSMPropertyHelper(getControlledProxy(), "ExclusionOffset").Get(offset, 3);
  vtkSMPropertyHelper(widget, "ExclusionOffset").Set(offset, 3);
//   std::cout << "Proxy Exclusion Offset: " << offset[0] << " " << offset[1]  << " " << offset[2] << std::endl;
  rrbw->SetExclusionOffset(offset);
  widget->UpdateVTKObjects();
  vtkPolyDataAlgorithm *region = vtkPolyDataAlgorithm::SafeDownCast(getControlledProxy()->GetClientSideObject());
  if (region)
  {
    vtkAbstractWidget *miwidget = widget->GetWidget();
    vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget)->SetVolume(region);
  }
}



//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::onWidgetVisibilityChanged(bool visible)
{
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::accept()
{
  std::cout << "Accept Changes"  << std::endl;
  updateControlledFilter();
//   vtkPolyDataAlgorithm *region = vtkPolyDataAlgorithm::SafeDownCast(getControlledProxy()->GetClientSideObject());
//   if (region)
//   {
//     vtkAbstractWidget *miwidget = widget->GetWidget();
//     vtkRectangularBoundingVolumeWidget::SafeDownCast(miwidget)->SetVolume(region);
//   }
  this->Superclass::accept();
//   this->hideHandles();
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::reset()
{
  std::cout << "Reset Changes"  << std::endl;
  updateWidgetMargins();
  this->Superclass::reset();
//   this->hideHandles();
}

//-----------------------------------------------------------------------------
void pqRectangularBoundingVolumeWidget::showHandles()
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
void pqRectangularBoundingVolumeWidget::hideHandles()
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
