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


#include "pqSliceView.h"

#include <QDebug>

// Server Manager Includes.
#include "QVTKWidget.h"
#include "vtkPVDataInformation.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTwoDRenderViewProxy.h"
#include "vtkSMSliceViewProxy.h"

// Qt Includes.
#include <QPushButton>
#include <QVBoxLayout>

// ParaView Includes.
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqDataRepresentation.h"
#include <vtkSMPropertyHelper.h>


pqSliceView::ManipulatorType pqSliceView::DefaultManipulatorTypes[9] =
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
pqSliceView::pqSliceView(
  const QString& group,
  const QString& name, 
  vtkSMViewProxy* viewProxy,
  pqServer* server, 
  QObject* _parent):
  Superclass(espinaRenderViewType(), group, name, viewProxy, server, _parent)
  , SlicingPlane(vtkPVSliceView::AXIAL)
{
  qDebug() << this << ": Created";
  this->InitializedWidgets = false;
}


//-----------------------------------------------------------------------------
pqSliceView::pqSliceView(
  const QString& viewtypemodule,
  const QString& group,
  const QString& name,
  vtkSMViewProxy* viewmodule,
  pqServer* server,
  QObject* p)
: Superclass(espinaRenderViewType(), group, name, viewmodule, server, p)
, SlicingPlane(vtkPVSliceView::AXIAL)
{
  qDebug() << "pqSliceView(" << this << ") : Created";
  this->InitializedWidgets = false;
}


//-----------------------------------------------------------------------------
pqSliceView::~pqSliceView()
{
}

//-----------------------------------------------------------------------------
/// Resets the camera to include all visible data.
/// It is essential to call this resetCamera, to ensure that the reset camera
/// action gets pushed on the interaction undo stack.
void pqSliceView::resetCamera()
{
  this->getProxy()->InvokeCommand("ResetCamera");
  this->render();
}

//-----------------------------------------------------------------------------
// This method is called for all pqSliceView objects irrespective
// of whether it is created from state/undo-redo/python or by the GUI. Hence
// don't change any render module properties here.
void pqSliceView::initializeWidgets()
{
  if (this->InitializedWidgets)
    {
    return;
    }

  this->InitializedWidgets = true;

  vtkSMSliceViewProxy* renModule = vtkSMSliceViewProxy::SafeDownCast(
    this->getProxy());

  QVTKWidget* vtkwidget = qobject_cast<QVTKWidget*>(this->getWidget());
  if (vtkwidget)
    {
    vtkwidget->SetRenderWindow(renModule->GetRenderWindow());
    }
}

//-----------------------------------------------------------------------------
vtkImageData* pqSliceView::captureImage(int magnification)
{
  Q_ASSERT(false);
  if (this->getWidget()->isVisible())
    {
    vtkSMRenderViewProxy* view = vtkSMRenderViewProxy::SafeDownCast(
      this->getProxy());

    return view->CaptureWindow(magnification);
    }

  // Don't return any image when the view is not visible.
  return NULL;
}

//-----------------------------------------------------------------------------
bool pqSliceView::canDisplay(pqOutputPort* opPort) const
{
  if (opPort == NULL || !this->Superclass::canDisplay(opPort))
    {
    return false;
    }

  pqPipelineSource* source = opPort->getSource();
  vtkSMSourceProxy* sourceProxy =
    vtkSMSourceProxy::SafeDownCast(source->getProxy());
  if (!sourceProxy ||
     sourceProxy->GetOutputPortsCreated()==0)
    {
    return false;
    }

  const char* dataclassname = opPort->getDataClassName();
  return (strcmp(dataclassname, "vtkImageData") == 0 ||
    strcmp(dataclassname, "vtkUniformGrid") == 0);
}

//-----------------------------------------------------------------------------
// void pqSliceView::setSlice(int pos)
// {
//   setSlice(static_cast<double>(pos));
// }


//-----------------------------------------------------------------------------
void pqSliceView::setSlice(double pos /*in nm*/)
{
//   qDebug() << this << ": Changing Slice " << pos;
  if (Center[SlicingPlane] == pos)
    return;

  Center[SlicingPlane] = pos;

  vtkSMPropertyHelper(this->getProxy(), "Center").Set(Center,3);
  this->getProxy()->UpdateVTKObjects();
  forceRender();

  emit centerChanged(Center[0], Center[1], Center[2]);
}

//-----------------------------------------------------------------------------
void pqSliceView::centerViewOn(double x, double y, double z)
{
//   qDebug() << "pqSliceView: Setting Center" << x << y << z;
  if (Center[0] == x && Center[1] == y && Center[2] == z)
    return;

  Center[0] = x;
  Center[1] = y;
  Center[2] = z;

  vtkSMPropertyHelper(this->getProxy(), "Center").Set(Center,3);
  this->getProxy()->UpdateVTKObjects();
  forceRender();

  emit centerChanged(Center[0], Center[1], Center[2]);
}


//-----------------------------------------------------------------------------
void pqSliceView::setSlicingPlane(vtkPVSliceView::VIEW_PLANE plane)
{
//   qDebug() << "Changing Slicing Plane";
  if (SlicingPlane == plane)
    return;

  SlicingPlane = plane;
  vtkSMPropertyHelper(this->getProxy(), "SlicingPlane").Set(plane);
  this->getProxy()->UpdateVTKObjects();
  forceRender();
}

//-----------------------------------------------------------------------------
void pqSliceView::setShowSegmentations(bool visible)
{
//   qDebug() << this << ": Segmentation Visibility = " << visible;
  vtkSMPropertyHelper(this->getProxy(), "ShowSegmentations").Set(visible);
  this->getProxy()->UpdateVTKObjects();
  forceRender();
}


// //-----------------------------------------------------------------------------
// void pqSliceView::updateVisibility(pqRepresentation* curRepr, bool visible)
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