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

class vtkImageActor;
class vtkActor;
class vtkPolyData;
class vtkMatrix4x4;
class vtkProp;
class vtkProp3D;
class vtkEspinaView;
class vtkLegendScaleActor;
class EspinaViewState;

class VTK_EXPORT vtkPVSliceView : public vtkPVRenderView
{
public:
    enum VIEW_PLANE
  {
    AXIAL = 2,
    SAGITTAL = 0,
    CORONAL = 1
  };

  struct SegActor
  {
    vtkImageActor *actor;
    double bounds[6];
  };

  static vtkPVSliceView* New();
  vtkTypeMacro(vtkPVSliceView, vtkPVRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddActor(vtkProp3D* actor);
  void AddChannel(vtkProp3D *actor);
  void AddSegmentation(SegActor *actor);


  // We need to reimplement the initilize method to overwrite
  // paraview's xml's default parameters
  virtual void Initialize(unsigned int id);
  // Reimplemented to update overview vtkPVRenderView behavior
  void ResetCamera();
  void ResetCamera(double bounds[6]);
  virtual void ResetCameraClippingRange();

  virtual void SetOrientationAxesVisibility(bool );

  void SetBackground(double r, double g, double b);

  vtkMatrix4x4 *GetSlicingMatrix() {return SlicingMatrix;}

  void SetSlice(double pos);
  vtkGetMacro(Slice, double);

  // Crosshair Related Methods
  void SetCenter(double x/*nm*/, double y/*nm*/, double z/*nm*/);
  void SetCenter(double center[3]/*nm*/);
  vtkGetVector3Macro(Center, double);

  void SetHCrossLineColor(double r, double g, double b);
  void SetHCrossLineColor(double color[3]);
  vtkGetVector3Macro(HCrossLineColor,double);

  void SetVCrossLineColor(double r, double g, double b);
  void SetVCrossLineColor(double color[3]);
  vtkGetVector3Macro(VCrossLineColor,double);

  void SetSlicingPlane(int plane);
  vtkGetMacro(SlicingPlane, int);

  void SetShowSegmentations(bool value);
  vtkGetMacro(ShowSegmentations, bool);

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

  vtkPVSliceView();
  ~vtkPVSliceView();

private:
  vtkPVSliceView(const vtkPVSliceView&); // Not implemented
  void operator=(const vtkPVSliceView&); // Not implemented

  EspinaViewState  *State;
  double           Slice;
  VIEW_PLANE       SlicingPlane;
  double           Center[3];
  bool             ShowSegmentations;
  double           HCrossLineColor[3];
  double           VCrossLineColor[3];
  double           SagittalCrossLineColor[3];
  double           CoronalCrossLineColor[3];

  vtkSmartPointer<vtkRenderer> OverviewRenderer;
  QList<vtkProp3D *> Channels;
  QList<SegActor *> Segmentations;

  vtkMatrix4x4     *SlicingMatrix;
  vtkEspinaView    *EspinaView;

  vtkSmartPointer<vtkPolyData> HCrossLineData, VCrossLineData;
  vtkSmartPointer<vtkActor> HCrossLine, VCrossLine;
//ETX
};
#endif // VTKPVSLICEVIEW_H
