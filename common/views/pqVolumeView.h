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


#ifndef PQVOLUMEVIEW_H
#define PQVOLUMEVIEW_H

#include <pqRenderViewBase.h>

class vtkSMRenderViewProxy;
/// pqRenderViewBase specialization used for espina
class PQCORE_EXPORT pqVolumeView : public pqRenderViewBase
{
    Q_OBJECT
    typedef pqRenderViewBase Superclass;
public:
    static QString espinaRenderViewType()
    {
        return "pqVolumeView";
    }
    static QString espinaRenderViewTypeName()
    {
        return "pqVolume View";
    }

    pqVolumeView ( const QString& group,
                  const QString& name,
                  vtkSMViewProxy* renModule,
                  pqServer* server,
                  QObject* parent=NULL );

    pqVolumeView ( const QString& viewtypemodule,
                  const QString& group,
                  const QString& name,
                  vtkSMViewProxy* viewmodule,
                  pqServer* server,
                  QObject* p );

    virtual ~pqVolumeView();

    /// Returns a array of 9 ManipulatorType objects defining
    /// default set of camera manipulators used by this type of view.
    /// There are exactly 9 entires in the returned array. It's is deliberately
    /// returned as non-constant. Developers can change the default by directly
    /// updating the entries.
    static ManipulatorType* getDefaultManipulatorTypes()
    {
        return pqVolumeView::DefaultManipulatorTypes;
    }

    /// Resets the camera to include all visible data.
    /// It is essential to call this resetCamera, to ensure that the reset camera
    /// action gets pushed on the interaction undo stack.
    virtual void resetCamera();

    /// Capture the view image into a new vtkImageData with the given magnification
    /// and returns it.
    virtual vtkImageData* captureImage ( int magnification );
    virtual vtkImageData* captureImage ( const QSize& size )
    {
        return this->Superclass::captureImage ( size );
    }

    /// returns whether a source can be displayed in this view module
    virtual bool canDisplay ( pqOutputPort* opPort ) const;

    virtual vtkSMRenderViewProxy *getRenderViewProxy() const;

public slots:
    /// Set the point in which the crosshair is centered on
    void setCrosshairCenter(double x/*nm*/, double y/*nm*/, double z/*nm*/ );
    /// Set the point in which the camera focus is centered on
    void setCameraFocus(double x/*nm*/, double y/*nm*/, double z/*nm*/ );
    /// Set whether segmentations are visible or not
    void setShowSegmentations ( bool visible );
    /// Set whether the scale ruler is visible or not
    void setRulerVisibility ( bool visible );


signals:
    void centerChanged ( double, double, double );

protected:
    /// Return the name of the group used for global settings (except interactor
    /// style).
    virtual const char* globalSettingsGroup() const
    {
        return "renderModule";
    }

    /// Return the name of the group used for view-sepecific settings such as
    /// background color, lighting.
    virtual const char* viewSettingsGroup() const
    {
        return "renderModulepqVolume";
    }

    /// Returns the name of the group in which to save the interactor style
    /// settings.
    virtual const char* interactorStyleSettingsGroup() const
    {
        return "renderModulepqVolume/InteractorStyle";
    }

    /// Must be overridden to return the default manipulator types.
    virtual const ManipulatorType* getDefaultManipulatorTypesInternal()
    {
        return pqVolumeView::getDefaultManipulatorTypes();
    }

    /// Setups up RenderModule and QVTKWidget binding.
    /// This method is called for all pqVolumeView objects irrespective
    /// of whether it is created from state/undo-redo/python or by the GUI. Hence
    /// don't change any render module properties here.
    virtual void initializeWidgets();
private:
    pqVolumeView ( const pqVolumeView& ); // Not implemented.
    void operator= ( const pqVolumeView& ); // Not implemented.

    static ManipulatorType DefaultManipulatorTypes[9];
    bool InitializedWidgets;

    double Crosshair[3];
    double Focus[3];
};

#endif // PQVOLUMEVIEW_H
