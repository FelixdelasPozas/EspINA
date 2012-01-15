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
    AXIAL,
    SAGITTAL,
    CORONAL
  };

  static vtkPVSliceView* New();
  vtkTypeMacro(vtkPVSliceView, vtkPVRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddActor(vtkProp3D* actor);
  void AddChannel(vtkProp3D *actor);
  void AddSegmentation(vtkProp3D *actor);


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

  void SetSlice(int value);
  vtkGetMacro(Slice, int);

  void SetCenter(double pos[3]/*in nm*/);

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

  vtkSmartPointer<vtkRenderer> OverviewRenderer;
protected:

//BTX
protected:
  vtkPVSliceView();
  ~vtkPVSliceView();

private:
  vtkPVSliceView(const vtkPVSliceView&); // Not implemented
  void operator=(const vtkPVSliceView&); // Not implemented

  EspinaViewState  *State;
  int              Slice;
  VIEW_PLANE       SlicingPlane;
  double           Center[3];
  bool             ShowSegmentations;

  QList<vtkProp3D *> Channels;
  QList<vtkProp3D *> Segmentations;

  vtkMatrix4x4     *SlicingMatrix;
  vtkEspinaView    *EspinaView;
//ETX
};
#endif // VTKPVSLICEVIEW_H
