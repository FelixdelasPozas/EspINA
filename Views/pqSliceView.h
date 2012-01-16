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


#ifndef PQSLICEVIEW_H
#define PQSLICEVIEW_H

#include <pqRenderViewBase.h>
#include "vtkPVSliceView.h"

/// pqRenderViewBase specialization used for espina
class PQCORE_EXPORT pqSliceView : public pqRenderViewBase
{
  Q_OBJECT
  typedef pqRenderViewBase Superclass;
public:
  static QString espinaRenderViewType() { return "pqSliceView"; }
  static QString espinaRenderViewTypeName() { return "pqSlice View"; }

  pqSliceView(const QString& group,
	     const QString& name,
	     vtkSMViewProxy* renModule,
	     pqServer* server,
	     QObject* parent=NULL);
  
  pqSliceView(const QString& viewtypemodule,
	     const QString& group,
	     const QString& name,
	     vtkSMViewProxy* viewmodule,
	     pqServer* server,
	     QObject* p);
  
  virtual ~pqSliceView();

// //     virtual QWidget* getWidget();
// 
  /// Returns a array of 9 ManipulatorType objects defining
  /// default set of camera manipulators used by this type of view.
  /// There are exactly 9 entires in the returned array. It's is deliberately
  /// returned as non-constant. Developers can change the default by directly
  /// updating the entries.
  static ManipulatorType* getDefaultManipulatorTypes()
    { return pqSliceView::DefaultManipulatorTypes; }

  /// Resets the camera to include all visible data.
  /// It is essential to call this resetCamera, to ensure that the reset camera
  /// action gets pushed on the interaction undo stack.
  virtual void resetCamera();

  /// Capture the view image into a new vtkImageData with the given magnification
  /// and returns it.
  virtual vtkImageData* captureImage(int magnification);
  virtual vtkImageData* captureImage(const QSize& size)
    { return this->Superclass::captureImage(size); }

  /// returns whether a source can be displayed in this view module
  virtual bool canDisplay(pqOutputPort* opPort) const;

  /// Set the type of the slicing plane
  void setSlicingPlane(vtkPVSliceView::VIEW_PLANE plane);

public slots:
  /// Set/Get the position in the slicing plane's normal direction in
  /// which the slice is obtained
  void setSlice(double pos /*nm*/);
  /// Set/Get the point in which the view is centered on
  void centerViewOn(double x/*nm*/, double y/*nm*/, double z/*nm*/);
  /// Set whether segmentations are visible or not
  void setShowSegmentations(bool visible);

signals:
  void centerChanged(double, double, double);

protected:
  /// Return the name of the group used for global settings (except interactor
  /// style).
  virtual const char* globalSettingsGroup() const
    { return "renderModule"; }

  /// Return the name of the group used for view-sepecific settings such as
  /// background color, lighting.
  virtual const char* viewSettingsGroup() const
  { return "renderModulepqSlice"; }

  /// Returns the name of the group in which to save the interactor style
  /// settings.
  virtual const char* interactorStyleSettingsGroup() const
  { return "renderModulepqSlice/InteractorStyle"; }

  /// Must be overridden to return the default manipulator types.
  virtual const ManipulatorType* getDefaultManipulatorTypesInternal()
  { return pqSliceView::getDefaultManipulatorTypes(); }

  /// Setups up RenderModule and QVTKWidget binding.
  /// This method is called for all pqSliceView objects irrespective
  /// of whether it is created from state/undo-redo/python or by the GUI. Hence
  /// don't change any render module properties here.
  virtual void initializeWidgets();
private:
  pqSliceView(const pqSliceView&); // Not implemented.
  void operator=(const pqSliceView&); // Not implemented.

  static ManipulatorType DefaultManipulatorTypes[9];
  bool InitializedWidgets;
  
  double Center[3];
  vtkPVSliceView::VIEW_PLANE SlicingPlane;
};

#endif // PQSLICEVIEW_H
