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


#ifndef VTKPVVOLUMEVIEW_H
#define VTKPVVOLUMEVIEW_H

#include <vtkPVRenderView.h>

#include <vtkSmartPointer.h>
#include <QList>
#include <QMap>
#include <vtkVolume.h>

class vtkLookupTable;
class vtkImageResliceToColors;
// Forward-declarations
class EspinaViewState;

class vtkActor;
class vtkAxisActor2D;
class vtkImageActor;
class vtkLegendScaleActor;
class vtkMatrix4x4;
class vtkPolyData;
class vtkProp3D;

class VTK_EXPORT vtkPVVolumeView : public vtkPVRenderView
{
public:
    struct VolumeActor
    {
      vtkVolume *prop;
      bool   visible;
      bool   selected;
      double bounds[6];
    };

    static vtkPVVolumeView* New();
    vtkTypeMacro ( vtkPVVolumeView, vtkPVRenderView );
    void PrintSelf ( ostream& os, vtkIndent indent );

    void AddActor (VolumeActor *actor);
    void RemoveActor(VolumeActor *actor);
    void AddChannel(VolumeActor *actor);
    void AddSegmentation(VolumeActor *actor);

    virtual void AddRepresentationInternal(vtkDataRepresentation* rep);
    virtual void RemoveRepresentationInternal(vtkDataRepresentation* rep);

//     vtkRenderer *GetOverviewRenderer();

    // We need to reimplement the initilize method to overwrite
    // paraview's xml's default parameters
    virtual void Initialize ( unsigned int id );
    // Reimplemented to update overview vtkPVRenderView behavior
    void ResetCamera();
    void ResetCamera ( double bounds[6] );
    virtual void ResetCameraClippingRange();

    virtual void SetOrientationAxesVisibility ( bool );

    void SetBackground ( double r, double g, double b );

    // Crosshair Related Methods
    void SetCenter ( double x/*nm*/, double y/*nm*/, double z/*nm*/ );
    void SetCenter ( double center[3]/*nm*/ );
    vtkGetVector3Macro ( Center, double );

    void SetHCrossLineColor ( double r, double g, double b );
    void SetHCrossLineColor ( double color[3] );
    vtkGetVector3Macro ( HCrossLineColor,double );

    void SetVCrossLineColor ( double r, double g, double b );
    void SetVCrossLineColor ( double color[3] );
    vtkGetVector3Macro ( VCrossLineColor,double );

    void SetShowSegmentations ( bool visible );
    vtkGetMacro ( ShowSegmentations, bool );

    void SetShowRuler ( bool visible );
    vtkGetMacro ( ShowRuler, bool );

    void SetRulerColor ( double r, double g, double b );
    void SetRulerColor ( double color[3] );
    vtkGetVector3Macro ( RulerColor,double );

    vtkSetVector2Macro ( RulerSize,double );
    vtkGetVector2Macro ( RulerSize,double );

//BTX
protected:
    void initCrosshairs();
    void initRuler();

    vtkPVVolumeView();
    ~vtkPVVolumeView();

private:
    vtkPVVolumeView ( const vtkPVVolumeView& ); // Not implemented
    void operator= ( const vtkPVVolumeView& ); // Not implemented

    void updateRuler();

    double           Center[3];
    bool             ShowSegmentations;

//     vtkSmartPointer<vtkRenderer> OverviewRenderer;
    QList<VolumeActor *> Channels;
    QList<VolumeActor *> Segmentations;

    vtkSmartPointer<vtkAxisActor2D> Ruler;
    bool             ShowRuler;
    double           RulerColor[3];
    double	   RulerSize[2];

    vtkSmartPointer<vtkPolyData>    HCrossLineData, VCrossLineData/*, BorderData*/;
    vtkSmartPointer<vtkActor>       HCrossLine, VCrossLine/*, Border*/;
    double           HCrossLineColor[3];
    double           VCrossLineColor[3];
    double           SagittalCrossLineColor[3];
    double           CoronalCrossLineColor[3];
    QMap<vtkDataRepresentation *, VolumeActor *> m_reps;
    VolumeActor        * m_pendingActor;
    QList<VolumeActor *> m_actors;
//ETX
};
#endif // VTKPVVOLUMEVIEW_H
