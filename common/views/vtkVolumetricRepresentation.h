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

//----------------------------------------------------------------------------
// File:    vtkVolumetricRepresentation.h
// Purpose: Transform source's vtkImageData into slice representations to
//          be rendered in vtkPVEspinaViews
//          vtkVolumetricRepresentation is not meant to be used with
//          conventional vtkPVViews
//----------------------------------------------------------------------------
#ifndef VTKVOLUMETRICREPRESENTATION_H
#define VTKVOLUMETRICREPRESENTATION_H

#include <vtkPVDataRepresentation.h>


#include <vtkVolume.h>
#include <vtkPVVolumeView.h>
#include <vtkVolumeRayCastMapper.h>

class vtkImageResliceToColors;
class vtkImageActor;
class vtkImageSliceDataDeliveryFilter;
class vtkImageVolume;
class vtkImageProperty;
class vtkImageResliceMapper;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;


class vtkVolumetricRepresentation : public vtkPVDataRepresentation
{
public:
  static vtkVolumetricRepresentation* New();
  vtkTypeMacro (vtkVolumetricRepresentation, vtkPVDataRepresentation);

  // vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
  // typically called by the vtkView to request meta-data from the
  // representations or ask them to perform certain tasks e.g.
  // PrepareForRendering.
  virtual int ProcessViewRequest(vtkInformationRequestKey *request_type, vtkInformation *inInfo, vtkInformation *outInfo);

  virtual void SetColor(double color[3]);
  virtual void SetColor(double r, double g, double b);
  vtkGetVector3Macro(Color, double);

  void SetOpacity(double value);
  vtkGetMacro(Opacity, double);

  vtkSetVector3Macro(Position, int);
  vtkGetVector3Macro(Position, int);

  //BTX
  vtkProp3D *GetVolumetricProp();

protected:
  vtkVolumetricRepresentation();
  virtual ~vtkVolumetricRepresentation();

  // Fill input port information
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  // Subclasses should override this to connect inputs to the internal pipeline
  // as necessary. Since most representations are "meta-filters" (i.e. filters
  // containing other filters), you should create shallow copies of your input
  // before connecting to the internal pipeline. The convenience method
  // GetInternalOutputPort will create a cached shallow copy of a specified
  // input for you. The related helper functions GetInternalAnnotationOutputPort,
  // GetInternalSelectionOutputPort should be used to obtain a selection or
  // annotation port whose selections are localized for a particular input data object.
  virtual int RequestData (vtkInformation *, vtkInformationVector  **, vtkInformationVector *);

  virtual vtkSmartPointer<vtkLookupTable> lut();
  virtual void AddToView(vtkPVVolumeView *view);
  virtual bool AddToView(vtkView *view);
  virtual void SetVisibility(bool val);

  vtkTimeStamp DeliveryTimeStamp;

  vtkImageSliceDataDeliveryFilter *DeliveryFilter;
  vtkPiecewiseFunction            *OpacityFunction;
  vtkColorTransferFunction        *ColorFunction;
  vtkVolumeRayCastMapper          *Volumetric;
  vtkVolume                       *VolumetricProp;
  vtkImageData                    *VolumetricData;
  vtkPVVolumeView::VolumeActor     VolumetricActor;

protected:
  double Color[3];
  int    Position[3];
  double Opacity;
  //ETX
};


#endif // VTKVOLUMETRICREPRESENTATION_H
