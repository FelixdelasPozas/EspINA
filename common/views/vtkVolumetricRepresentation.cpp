/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "vtkVolumetricRepresentation.h"

#include <QDebug>

#include "vtkPVVolumeView.h"
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkPVLODActor.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>

#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkImageResliceToColors.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkImageSliceDataDeliveryFilter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPlane.h>
#include <vtkMetaImageWriter.h>
#include <vtkPointData.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

vtkStandardNewMacro ( vtkVolumetricRepresentation );

// static bool base = true;
//----------------------------------------------------------------------------
vtkVolumetricRepresentation::vtkVolumetricRepresentation()
{
  memset(Color, 0, 3*sizeof(int));
  memset(Position, 0, 3*sizeof(int));
  VolumetricData = vtkImageData::New();

  DeliveryFilter = vtkImageSliceDataDeliveryFilter::New();
  Volumetric = vtkVolumeRayCastMapper::New();
  Volumetric->SetVolumeRayCastFunction(vtkVolumeRayCastCompositeFunction::New());
  VolumetricProp = vtkVolume::New();
  VolumetricProp->SetPickable(true);
  vtkVolumeProperty *volProperty = VolumetricProp->GetProperty();

  OpacityFunction = vtkPiecewiseFunction::New();
  OpacityFunction->AddPoint(0, 0.0);
  OpacityFunction->AddPoint(1, 1.0);
  volProperty->SetScalarOpacity(OpacityFunction);

  ColorFunction = vtkColorTransferFunction::New();
  ColorFunction->AddRGBSegment(0.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0);
  volProperty->SetColor(ColorFunction);

  volProperty->ShadeOn();

  Volumetric->ReleaseDataFlagOn();
  DeliveryFilter->ReleaseDataFlagOn();

  VolumetricActor.prop = NULL;
  VolumetricProp->SetMapper(Volumetric);
//   qDebug() << "Created Representation" << this;
}

//----------------------------------------------------------------------------
vtkVolumetricRepresentation::~vtkVolumetricRepresentation()
{
  VolumetricData->Delete();
  DeliveryFilter->RemoveAllInputs();;
  DeliveryFilter->Delete();
  Volumetric->RemoveAllInputs();
  Volumetric->Delete();
  // WARNING: DO NOT FREE ACTOR, BECAUSE IT'S FREED BY THE RENDERER
  // VolumetricActor->GetMapper()->Delete();
  // VolumetricActor->Delete();
//   qDebug() << "Destroyed Representation" << this;
}

//----------------------------------------------------------------------------
// This method has been taken from vtkImageVolumetricRepresentation and modified
// to fit our purpose
int vtkVolumetricRepresentation::ProcessViewRequest (
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo,
  vtkInformation* outInfo )
{
  if ( request_type == vtkPVView::REQUEST_INFORMATION() )
  {
    // Here we need to tell the view about the geometry size and if we need
    // ordered compositing.
    vtkDataObject* slice = this->VolumetricData;
    if ( slice )
    {
      outInfo->Set ( vtkPVRenderView::GEOMETRY_SIZE(), slice->GetActualMemorySize() );
    }
  }
  else if ( request_type == vtkPVView::REQUEST_PREPARE_FOR_RENDER() )
  {
    // In REQUEST_PREPARE_FOR_RENDER, we need to ensure all our data-deliver
    // filters have their states updated as requested by the view.
    this->DeliveryFilter->ProcessViewRequest ( inInfo );

    // we have to use DeliveryTimeStamp since when image-data has invalid
    // extents the executive goes berserk and always keeps on re-executing the
    // pipeline which breaks when running in parallel.
    if ( this->DeliveryTimeStamp < this->DeliveryFilter->GetMTime() )
    {
      outInfo->Set ( vtkPVRenderView::NEEDS_DELIVERY(), 1 );
    }
  }
  else if ( request_type == vtkPVView::REQUEST_DELIVERY() )
  {
    this->DeliveryFilter->Modified();
    this->DeliveryFilter->Update();

    // since there's no direct connection between the mapper and the collector,
    // we don't put an update-suppressor in the pipeline.

    // essential to break the pipeline link between the mapper and the delivery
    // filter since when the delivery filter produces an empty image, the
    // executive keeps on re-executing it every time.
    vtkImageData* clone = vtkImageData::New();
    clone->ShallowCopy ( this->DeliveryFilter->GetOutputDataObject ( 0 ) );
//     clone->CopyInformation(this->DeliveryFilter->GetOutputDataObject ( 0 ) );
    this->Volumetric->SetInput ( clone );
    clone->Delete();

    this->DeliveryTimeStamp.Modified();
  }
  else if ( request_type == vtkPVView::REQUEST_RENDER() )
  {
    // this->VolumetricMapper->Update();
  }

  return this->Superclass::ProcessViewRequest ( request_type, inInfo, outInfo );
}

//----------------------------------------------------------------------------
int vtkVolumetricRepresentation::FillInputPortInformation ( int port, vtkInformation* info )
{
  info->Set ( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData" );
  info->Set ( vtkAlgorithm::INPUT_IS_OPTIONAL(), 1 );
  return 1;
}

//----------------------------------------------------------------------------
int vtkVolumetricRepresentation::RequestData (
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector )
{
  // Mark delivery filter modified.
  this->DeliveryFilter->Modified();

  // Pass caching information to the cache keeper.
  //   this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  //   this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if ( inputVector[0]->GetNumberOfInformationObjects() ==1 )
  {
//     vtkInformation* inInfo = inputVector[0]->GetInformationObject ( 0 );
    vtkImageData* input = vtkImageData::GetData ( inputVector[0], 0 );

    vtkImageData *clone = vtkImageData::New();
    clone->ShallowCopy ( input );
    Volumetric->SetInput ( clone );
    Volumetric->Update();
    this->VolumetricData->ShallowCopy ( clone );
    clone->Delete();
    input->GetBounds(VolumetricActor.bounds);
    this->DeliveryFilter->SetInput ( input );
  }
  else
  {
    this->DeliveryFilter->RemoveAllInputs();
  }

  return this->Superclass::RequestData ( request, inputVector, outputVector );
}


//----------------------------------------------------------------------------
vtkSmartPointer< vtkLookupTable > vtkVolumetricRepresentation::lut()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Build();
  return lut;
}

//----------------------------------------------------------------------------
void vtkVolumetricRepresentation::AddToView(vtkPVVolumeView* view)
{
  view->AddSegmentation(&VolumetricActor);
}

//----------------------------------------------------------------------------
bool vtkVolumetricRepresentation::AddToView(vtkView* view)
{
  //   qDebug() << "Add to View";
  vtkPVVolumeView* rview = vtkPVVolumeView::SafeDownCast(view);
  if (rview)
  {
//     Volumetric->SetOutputDimensionality(2);
//     Volumetric->SetResliceAxes(rview->GetSlicingMatrix());
    VolumetricActor.prop = VolumetricProp;
//     VolumetricActor.mapper = Volumetric;

//     VolumetricActor.prop->SetOpacity(Opacity);
//     VolumetricActor.prop->SetPosition(Position[0],Position[1],Position[2]);
    AddToView(rview);


    return true;
  }else
    return false;
}
//----------------------------------------------------------------------------
void vtkVolumetricRepresentation::SetVisibility(bool val)
{
  VolumetricProp->SetVisibility(val);
  vtkPVDataRepresentation::SetVisibility(val);
  VolumetricActor.visible = val;
}

//----------------------------------------------------------------------------
void vtkVolumetricRepresentation::SetColor(double color[3])
{
  SetColor(color[0], color[1], color[2]);
}

//----------------------------------------------------------------------------
void vtkVolumetricRepresentation::SetColor(double r, double g, double b)
{
  Color[0] = r;
  Color[1] = g;
  Color[2] = b;

  ColorFunction->AddRGBPoint(1.0, r, g, b);
}

//----------------------------------------------------------------------------
void vtkVolumetricRepresentation::SetOpacity(double value)
{
  if (Opacity == value)
    return;

  Opacity = value;
  if (ColorFunction)
  {
    ColorFunction->SetAlpha(value);
    ColorFunction->Modified();
  }

  Modified();
}

//----------------------------------------------------------------------------
vtkProp3D* vtkVolumetricRepresentation::GetVolumetricProp()
{
  return VolumetricProp;
}
