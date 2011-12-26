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


#ifndef VTKPVESPINAVIEW_H
#define VTKPVESPINAVIEW_H

#include <vtkPVRenderView.h>

#include <vtkSmartPointer.h>

class vtkProp;
class vtkEspinaView;
class vtkLegendScaleActor;
class VTK_EXPORT vtkPVEspinaView : public vtkPVRenderView
{
public:
  static vtkPVEspinaView* New();
  vtkTypeMacro(vtkPVEspinaView, vtkPVRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddActor(vtkProp *actor);

  // Reimplemented to update overview vtkPVRenderView behavior
  void ResetCamera();
  void ResetCamera(double bounds[6]);
  virtual void ResetCameraClippingRange();

  virtual void SetOrientationAxesVisibility(bool );

  void SetBackground(double r, double g, double b);

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
  vtkPVEspinaView();
  ~vtkPVEspinaView();

private:
  vtkPVEspinaView(const vtkPVEspinaView&); // Not implemented
  void operator=(const vtkPVEspinaView&); // Not implemented

  vtkEspinaView *EspinaView;
//ETX
};
#endif // VTKPVESPINAVIEW_H
