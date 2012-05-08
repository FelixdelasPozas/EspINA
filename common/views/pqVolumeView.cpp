/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "pqVolumeView.h"

#include <QDebug>

// Server Manager Includes.
#include "QVTKWidget.h"
#include "vtkPVDataInformation.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTwoDRenderViewProxy.h"

// Qt Includes.
#include <QPushButton>
#include <QVBoxLayout>

// ParaView Includes.
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqDataRepresentation.h"
#include <vtkSMPropertyHelper.h>


pqVolumeView::ManipulatorType pqVolumeView::DefaultManipulatorTypes[9] =
{
    { 1, 0, 0, "Pan"},
    { 2, 0, 0, "Pan"},
    { 3, 0, 0, "Zoom"},
    { 1, 1, 0, "Zoom"},
    { 2, 1, 0, "Zoom"},
    { 3, 1, 0, "Zoom"},
    { 1, 0, 1, "Zoom"},
    { 2, 0, 1, "Zoom"},
    { 3, 0, 1, "Pan"},
};


//-----------------------------------------------------------------------------
pqVolumeView::pqVolumeView (
    const QString& group,
    const QString& name,
    vtkSMViewProxy* viewProxy,
    pqServer* server,
    QObject* _parent ) :
        Superclass ( espinaRenderViewType(), group, name, viewProxy, server, _parent )
{
    qDebug() << this << ": Created";
    this->InitializedWidgets = false;
}


//-----------------------------------------------------------------------------
pqVolumeView::pqVolumeView (
    const QString& viewtypemodule,
    const QString& group,
    const QString& name,
    vtkSMViewProxy* viewmodule,
    pqServer* server,
    QObject* p )
        : Superclass ( espinaRenderViewType(), group, name, viewmodule, server, p )
{
    qDebug() << "pqVolumeView(" << this << ") : Created";
    this->InitializedWidgets = false;
}


//-----------------------------------------------------------------------------
pqVolumeView::~pqVolumeView()
{
}

//-----------------------------------------------------------------------------
/// Resets the camera to include all visible data.
/// It is essential to call this resetCamera, to ensure that the reset camera
/// action gets pushed on the interaction undo stack.
void pqVolumeView::resetCamera()
{
    this->getProxy()->InvokeCommand ( "ResetCamera" );
    this->render();
}

//-----------------------------------------------------------------------------
// This method is called for all pqVolumeView objects irrespective
// of whether it is created from state/undo-redo/python or by the GUI. Hence
// don't change any render module properties here.
void pqVolumeView::initializeWidgets()
{
    if ( this->InitializedWidgets )
    {
        return;
    }

    this->InitializedWidgets = true;

    vtkSMRenderViewProxy* renModule = vtkSMRenderViewProxy::SafeDownCast (
                                         this->getProxy() );

    QVTKWidget* vtkwidget = qobject_cast<QVTKWidget*> ( this->getWidget() );
    if ( vtkwidget )
    {
        vtkwidget->SetRenderWindow ( renModule->GetRenderWindow() );
    }
}

//-----------------------------------------------------------------------------
vtkImageData* pqVolumeView::captureImage ( int magnification )
{
    if ( this->getWidget()->isVisible() )
    {
        vtkSMRenderViewProxy* view = vtkSMRenderViewProxy::SafeDownCast (
                                         this->getProxy() );

        return view->CaptureWindow ( magnification );
    }

    // Don't return any image when the view is not visible.
    return NULL;
}

//-----------------------------------------------------------------------------
bool pqVolumeView::canDisplay ( pqOutputPort* opPort ) const
{
    if ( opPort == NULL || !this->Superclass::canDisplay ( opPort ) )
    {
        return false;
    }

    pqPipelineSource* source = opPort->getSource();
    vtkSMSourceProxy* sourceProxy =
        vtkSMSourceProxy::SafeDownCast ( source->getProxy() );
    if ( !sourceProxy ||
            sourceProxy->GetOutputPortsCreated() ==0 )
    {
        return false;
    }

    const char* dataclassname = opPort->getDataClassName();
    return ( strcmp ( dataclassname, "vtkImageData" ) == 0 ||
             strcmp ( dataclassname, "vtkUniformGrid" ) == 0 );
}

//-----------------------------------------------------------------------------
// void pqVolumeView::setVolume(int pos)
// {
//   setVolume(static_cast<double>(pos));
// }

//-----------------------------------------------------------------------------
void pqVolumeView::setCrosshairCenter(double x, double y, double z)
{
  if ( Crosshair[0] == x && Crosshair[1] == y && Crosshair[2] == z )
    return;

  Crosshair[0] = x;
  Crosshair[1] = y;
  Crosshair[2] = z;

  vtkSMPropertyHelper(this->getProxy(), "Crosshair").Set (Crosshair, 3);
  this->getProxy()->UpdateVTKObjects();
  forceRender();

//   emit centerChanged(Crosshair[0], Crosshair[1], Crosshair[2]);
}

//-----------------------------------------------------------------------------
void pqVolumeView::setCameraFocus(double x, double y, double z)
{
  if (Focus[0] == x && Focus[1] == y && Focus[2] == z)
    return;

  Focus[0] = x;
  Focus[1] = y;
  Focus[2] = z;

  vtkSMPropertyHelper(this->getProxy(), "Focus").Set (Focus, 3);
  this->getProxy()->UpdateVTKObjects();
  forceRender();
//   emit centerChanged(Crosshair[0], Crosshair[1], Crosshair[2]);
}

//-----------------------------------------------------------------------------
vtkSMRenderViewProxy* pqVolumeView::getRenderViewProxy() const
{
  return vtkSMRenderViewProxy::SafeDownCast(this->getViewProxy());
}

//-----------------------------------------------------------------------------
void pqVolumeView::setShowSegmentations ( bool visible )
{
//   qDebug() << this << ": Segmentation Visibility = " << visible;
    vtkSMPropertyHelper ( this->getProxy(), "ShowSegmentations" ).Set ( visible );
    this->getProxy()->UpdateVTKObjects();
    forceRender();
}

//-----------------------------------------------------------------------------
void pqVolumeView::setRulerVisibility ( bool visible )
{
    vtkSMPropertyHelper ( this->getProxy(), "ShowRuler" ).Set ( visible );
    this->getProxy()->UpdateVTKObjects();
    forceRender();
}


// //-----------------------------------------------------------------------------
// void pqVolumeView::updateVisibility(pqRepresentation* curRepr, bool visible)
// {
//   if (!qobject_cast<pqDataRepresentation*>(curRepr))
//   {
//     return;
//   }
/*
  if (visible)
  {
    QList<pqRepresentation*> reprs = this->getRepresentations();
    foreach (pqRepresentation* repr, reprs)
    {
      if (!qobject_cast<pqDataRepresentation*>(repr))
      {
	continue;
      }
      if (repr != curRepr && repr->isVisible())
      {
	repr->setVisible(false);
      }
    }
  }
}*/
