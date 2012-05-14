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


#include "vtkCrosshairRepresentation.h"

#include <QDebug>

#include "vtkPVVolumeView.h"

#include <vtkAssembly.h>
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
#include <vtkMatrix4x4.h>

vtkStandardNewMacro ( vtkCrosshairRepresentation );

// static bool base = true;
//----------------------------------------------------------------------------
vtkCrosshairRepresentation::vtkCrosshairRepresentation()
: Color(0.0)
{
  memset(Position, 0, 3*sizeof(int));

  CrosshairProp = vtkAssembly::New();

  this->DeliveryFilter = vtkImageSliceDataDeliveryFilter::New();
  for (int i = 0; i < 3; i++)
  {
    this->CrosshairData[i] = vtkImageData::New();
    this->Crosshair[i] = vtkImageResliceToColors::New();
    this->CrosshairSliceProp[i] = vtkImageActor::New();
    this->CrosshairSliceProp[i]->GetMapper()->BorderOn();
    this->CrosshairSliceProp[i]->SetInterpolate(false);
    this->CrosshairSliceProp[i]->GetMapper()->SetInputConnection(Crosshair[i]->GetOutputPort());
    this->Crosshair[i]->ReleaseDataFlagOn();
    CrosshairProp->AddPart(CrosshairSliceProp[i]);
  }
  DeliveryFilter->ReleaseDataFlagOn();

  CrosshairActor.prop = NULL;
//   qDebug() << "Created Representation" << this;
}

//----------------------------------------------------------------------------
vtkCrosshairRepresentation::~vtkCrosshairRepresentation()
{
  DeliveryFilter->RemoveAllInputs();;
  DeliveryFilter->Delete();
  for(int i=0; i<3; i++)
  {
    CrosshairData[i]->Delete();
    Crosshair[i]->RemoveAllInputs();
    Crosshair[i]->Delete();
  }
  // WARNING: DO NOT FREE ACTOR, BECAUSE IT'S FREED BY THE RENDERER
  // CrosshairActor->GetMapper()->Delete();
  // CrosshairActor->Delete();
//   qDebug() << "Destroyed Representation" << this;
}

//----------------------------------------------------------------------------
// This method has been taken from vtkImageCrosshairRepresentation and modified
// to fit our purpose
int vtkCrosshairRepresentation::ProcessViewRequest (
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo,
  vtkInformation* outInfo )
{
  if ( request_type == vtkPVView::REQUEST_INFORMATION() )
  {
    // Here we need to tell the view about the geometry size and if we need
    // ordered compositing.
    vtkDataObject* slice = this->CrosshairData[0]; //TODO: Add memory of all slices
    if (slice)
    {
      outInfo->Set ( vtkPVRenderView::GEOMETRY_SIZE(), slice->GetActualMemorySize() );
    }
    if ( this->CrosshairSliceProp[0]->GetOpacity() < 1.0 )
    {
      outInfo->Set ( vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1 );
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
    this->Crosshair[0]->SetInput(clone);
    this->Crosshair[1]->SetInput(clone);
    this->Crosshair[2]->SetInput(clone);
    clone->Delete();

    this->DeliveryTimeStamp.Modified();
  }
  else if ( request_type == vtkPVView::REQUEST_RENDER() )
  {
    // this->CrosshairMapper->Update();
  }

  return this->Superclass::ProcessViewRequest ( request_type, inInfo, outInfo );
}

//----------------------------------------------------------------------------
int vtkCrosshairRepresentation::FillInputPortInformation ( int port, vtkInformation* info )
{
  info->Set ( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData" );
  info->Set ( vtkAlgorithm::INPUT_IS_OPTIONAL(), 1 );
  return 1;
}

//----------------------------------------------------------------------------
int vtkCrosshairRepresentation::RequestData (
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

//     int e[6];
//     input->GetExtent(e);
//     std::cout << "Crosshair Rep Input has extent: " << e[0] << " " << e[1]<< " " << e[2]<< " " << e[3]<< " " << e[4]<< " " << e[5] << std::endl;
//     std::cout << "Number Of Tuples " << input->GetPointData()->GetNumberOfTuples() << std::endl;

    vtkImageData *clone = vtkImageData::New();
    clone->ShallowCopy ( input );
//     clone->CopyInformation(input);
//     clone->GetExtent(e);
    for (int i=0; i < 3; i++)
    {
      Crosshair[i]->SetInput(clone);
      Crosshair[i]->Update();
      this->CrosshairData[i]->ShallowCopy(Crosshair[i]->GetOutput());
    }
    clone->Delete();
    input->GetBounds(CrosshairActor.bounds);
    this->DeliveryFilter->SetInput(input);
  }
  else
  {
    this->DeliveryFilter->RemoveAllInputs();
  }

  return this->Superclass::RequestData ( request, inputVector, outputVector );
}


//----------------------------------------------------------------------------
vtkSmartPointer< vtkLookupTable > vtkCrosshairRepresentation::lut()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Build();
  return lut;
}

//----------------------------------------------------------------------------
void vtkCrosshairRepresentation::AddToView(vtkPVVolumeView* view)
{
  view->AddChannel(&CrosshairActor);
}

//----------------------------------------------------------------------------
bool vtkCrosshairRepresentation::AddToView(vtkView* view)
{
  //   qDebug() << "Add to View";
  vtkPVVolumeView* rview = vtkPVVolumeView::SafeDownCast(view);
  if (rview)
  {
    Crosshair[0]->SetOutputDimensionality(2);
    Crosshair[0]->SetResliceAxes(rview->AxialSlicingMatrix());
    CrosshairSliceProp[0]->SetPickable(true);
    Crosshair[1]->SetOutputDimensionality(2);
    Crosshair[1]->SetResliceAxes(rview->SagittalSlicingMatrix());
    CrosshairSliceProp[1]->SetPickable(true);
    CrosshairSliceProp[1]->RotateX(-90);
    CrosshairSliceProp[1]->RotateY(-90);
    Crosshair[2]->SetOutputDimensionality(2);
    Crosshair[2]->SetResliceAxes(rview->CoronalSlicingMatrix());
    CrosshairSliceProp[2]->SetPickable(true);
    CrosshairSliceProp[2]->RotateX(-90);

    CrosshairActor.prop = CrosshairProp;
//     CrosshairActor.mapper = Crosshair;

//     CrosshairActor.lut = lut();
//     Crosshair->SetLookupTable(CrosshairActor.lut);

//     CrosshairActor.prop->SetOpacity(Opacity);
    CrosshairActor.prop->SetPosition(Position[0],Position[1],Position[2]);
    AddToView(rview);


    return true;
  }else
    return false;
}
//----------------------------------------------------------------------------
void vtkCrosshairRepresentation::SetVisibility(bool val)
{
  CrosshairProp->SetVisibility(val);
  vtkPVDataRepresentation::SetVisibility(val);
  CrosshairActor.visible = val;
}

//----------------------------------------------------------------------------
void vtkCrosshairRepresentation::SetColor(double color)
{
  Color = color;
//   if (CrosshairActor.lut == NULL)
//     return;

  double saturation = Color>=0?1.0:0;
//   CrosshairActor.lut->SetSaturationRange(0.0, saturation);
//   CrosshairActor.lut->SetHueRange(Color, Color);
//   CrosshairActor.lut->Build();
}

//----------------------------------------------------------------------------
void vtkCrosshairRepresentation::SetOpacity(double value)
{
//   if (Opacity == value)
//     return;

  Opacity = value;
//   if (CrosshairActor.prop)
//     CrosshairActor.prop->SetOpacity(value);
  Modified();
}

//----------------------------------------------------------------------------
vtkProp3D* vtkCrosshairRepresentation::GetCrosshairProp()
{
  return CrosshairProp;
}
