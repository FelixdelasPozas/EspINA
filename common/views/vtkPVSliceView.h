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


#ifndef VTKPVSLICEVIEW_H
#define VTKPVSLICEVIEW_H

#include <vtkPVRenderView.h>

#include <vtkSmartPointer.h>
#include <QList>
#include <QMap>

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

class VTK_EXPORT vtkPVSliceView : public vtkPVRenderView
{
public:
    enum VIEW_PLANE
    {
        AXIAL = 2,
        SAGITTAL = 0,
        CORONAL = 1
    };

    struct SliceActor
    {
      vtkSmartPointer<vtkLookupTable> lut;
      vtkImageResliceToColors *mapper;
      vtkImageActor *prop;
      bool   visible;
      bool   selected;
      double bounds[6];
    };

    static vtkPVSliceView* New();
    vtkTypeMacro ( vtkPVSliceView, vtkPVRenderView );
    void PrintSelf ( ostream& os, vtkIndent indent );

    void AddActor (SliceActor *actor);
    void RemoveActor(SliceActor *actor);
    void AddChannel(SliceActor *actor);
    void AddSegmentation(SliceActor *actor);

    virtual void AddRepresentationInternal(vtkDataRepresentation* rep);
    virtual void RemoveRepresentationInternal(vtkDataRepresentation* rep);

    vtkRenderer *GetOverviewRenderer();

    // We need to reimplement the initilize method to overwrite
    // paraview's xml's default parameters
    virtual void Initialize ( unsigned int id );
    // Reimplemented to update overview vtkPVRenderView behavior
    void ResetCamera();
    void ResetCamera ( double bounds[6] );
    virtual void ResetCameraClippingRange();

    virtual void SetOrientationAxesVisibility ( bool );

    void SetBackground ( double r, double g, double b );

    vtkMatrix4x4 *GetSlicingMatrix()
    {
        return SlicingMatrix;
    }

    void SetSlice ( double pos );
    vtkGetMacro ( Slice, double );

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

    void SetSlicingPlane ( int plane );
    vtkGetMacro ( SlicingPlane, int );

    void SetShowSegmentations ( bool visible );
    vtkGetMacro ( ShowSegmentations, bool );

    void SetShowRuler ( bool visible );
    vtkGetMacro ( ShowRuler, bool );

    void SetRulerColor ( double r, double g, double b );
    void SetRulerColor ( double color[3] );
    vtkGetVector3Macro ( RulerColor,double );

    vtkSetVector2Macro ( RulerSize,double );
    vtkGetVector2Macro ( RulerSize,double );
    // Method called from xml configuration when adding a new sample
    // to the view using the SMAdaptor:
    //  pqSMAdaptor::addProxyProperty(
    //  viewModuleProxy->GetProperty("Representations"), reprProxy); //change to Samples
    //  viewModuleProxy->UpdateVTKObjects();
//   void AddSample(vtkDataRepresentation *rep);
//   void RemoveSample(vtkDataRepresentation *rep);

//BTX
protected:
    void initCrosshairs();
    void initRuler();

    vtkPVSliceView();
    ~vtkPVSliceView();

private:
    vtkPVSliceView ( const vtkPVSliceView& ); // Not implemented
    void operator= ( const vtkPVSliceView& ); // Not implemented

    void updateRuler();

    EspinaViewState  *State;
    double           Slice;
    VIEW_PLANE       SlicingPlane;
    double           Center[3];
    bool             ShowSegmentations;

    vtkSmartPointer<vtkRenderer> OverviewRenderer;
    QList<SliceActor *> Channels;
    QList<SliceActor *> Segmentations;

    vtkMatrix4x4     *SlicingMatrix;

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
    QMap<vtkDataRepresentation *, SliceActor *> m_reps;
    SliceActor        * m_pendingActor;
    QList<SliceActor *> m_actors;
//ETX
};
#endif // VTKPVSLICEVIEW_H
