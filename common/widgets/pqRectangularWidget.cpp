/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "pqRectangularWidget.h"

#include "vtkRectangularWidget.h"

// Qt Includes.
#include <QDoubleValidator>

#include <pq3DWidgetFactory.h>
#include <pqApplicationCore.h>
#include <pqPropertyLinks.h>
#include <pqSMAdaptor.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>
#include "vtkRectangularRepresentation.h"

//-----------------------------------------------------------------------------
pqRectangularWidget::pqRectangularWidget(vtkSMProxy* refProxy, vtkSMProxy* pxy, QWidget* _parent) :
  Superclass(refProxy, pxy, _parent)
{
  QObject::connect(this, SIGNAL(widgetVisibilityChanged(bool)),
    this, SLOT(onWidgetVisibilityChanged(bool)));

  pqServerManagerModel* smmodel =
    pqApplicationCore::instance()->getServerManagerModel();
  this->createWidget(smmodel->findServer(refProxy->GetSession()));
}

//-----------------------------------------------------------------------------
pqRectangularWidget::~pqRectangularWidget()
{
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::createWidget(pqServer* server)
{
  vtkSMNewWidgetRepresentationProxy* widget =
    pqApplicationCore::instance()->get3DWidgetFactory()->
    get3DWidget("RectangularWidgetRepresentation", server);
  this->setWidgetProxy(widget);

  getControlledProxy()->UpdateSelfAndAllInputs();

  widget->UpdateVTKObjects();
  widget->UpdatePropertyInformation();
}

//-----------------------------------------------------------------------------
// update widget bounds.
void pqRectangularWidget::select()
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
void pqRectangularWidget::resetBounds(double input_bounds[6])
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  vtkSMPropertyHelper(widget, "PlaceWidget").Set(input_bounds, 6);
  widget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::cleanupWidget()
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
void pqRectangularWidget::updateControlledFilter()
{
//   std::cout << "Updating Controlled Filter" << std::endl;
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  Q_ASSERT(widget);
  vtkSMProxy *proxy = widget->GetRepresentationProxy();
  Q_ASSERT(proxy);
  vtkRectangularRepresentation *rep =
    dynamic_cast<vtkRectangularRepresentation *>(proxy->GetClientSideObject());
  Q_ASSERT(rep);
  vtkSMPropertyHelper(getControlledProxy(), "Bounds").Set(rep->GetBounds(), 6);
  getControlledProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::updateWidgetMargins()
{
//   std::cout << "Update Widget Margins" << std::endl;
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  getControlledProxy()->UpdatePropertyInformation();
  widget->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::onWidgetVisibilityChanged(bool visible)
{
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::accept()
{
//   std::cout << "Accept Changes"  << std::endl;
  updateControlledFilter();
  this->Superclass::accept();
}

//-----------------------------------------------------------------------------
void pqRectangularWidget::reset()
{
//   std::cout << "Reset Changes"  << std::endl;
  updateWidgetMargins();
  this->Superclass::reset();
}