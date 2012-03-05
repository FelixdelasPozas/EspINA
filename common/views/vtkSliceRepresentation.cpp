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


#include "vtkSliceRepresentation.h"

#include <QDebug>

#include "vtkPVSliceView.h"
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

vtkStandardNewMacro ( vtkSliceRepresentation );

static bool base = true;
//----------------------------------------------------------------------------
vtkSliceRepresentation::vtkSliceRepresentation()
: Color(0.0)
{
  bzero(Position,3*sizeof(int));
  this->SliceData = vtkImageData::New();

  this->DeliveryFilter = vtkImageSliceDataDeliveryFilter::New();
  this->Slice = vtkImageResliceToColors::New();
  this->SliceProp = vtkImageActor::New();
  this->SliceProp->SetInterpolate(false);
  this->SliceProp->GetMapper()->BorderOn();
  Slice->ReleaseDataFlagOn();
  DeliveryFilter->ReleaseDataFlagOn();

  SliceActor.prop = NULL;
  this->SliceProp->GetMapper()->SetInputConnection(Slice->GetOutputPort());
//   qDebug() << "Created Representation" << this;
}

//----------------------------------------------------------------------------
vtkSliceRepresentation::~vtkSliceRepresentation()
{
  SliceData->Delete();
  DeliveryFilter->RemoveAllInputs();;
  DeliveryFilter->Delete();
  Slice->RemoveAllInputs();
  Slice->Delete();
  // WARNING: DO NOT FREE ACTOR, BECAUSE IT'S FREED BY THE RENDERER
  // SliceActor->GetMapper()->Delete();
  // SliceActor->Delete();
//   qDebug() << "Destroyed Representation" << this;
}

//----------------------------------------------------------------------------
// This method has been taken from vtkImageSliceRepresentation and modified
// to fit our purpose
int vtkSliceRepresentation::ProcessViewRequest (
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo,
  vtkInformation* outInfo )
{
  if ( request_type == vtkPVView::REQUEST_INFORMATION() )
  {
    // Here we need to tell the view about the geometry size and if we need
    // ordered compositing.
    vtkDataObject* slice = this->SliceData;
    if ( slice )
    {
      outInfo->Set ( vtkPVRenderView::GEOMETRY_SIZE(), slice->GetActualMemorySize() );
    }
    if ( this->SliceProp->GetOpacity() < 1.0 )
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
    this->Slice->SetInput ( clone );
    clone->Delete();

    this->DeliveryTimeStamp.Modified();
  }
  else if ( request_type == vtkPVView::REQUEST_RENDER() )
  {
    // this->SliceMapper->Update();
  }

  return this->Superclass::ProcessViewRequest ( request_type, inInfo, outInfo );
}

//----------------------------------------------------------------------------
int vtkSliceRepresentation::FillInputPortInformation ( int port, vtkInformation* info )
{
  info->Set ( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData" );
  info->Set ( vtkAlgorithm::INPUT_IS_OPTIONAL(), 1 );
  return 1;
}

//----------------------------------------------------------------------------
int vtkSliceRepresentation::RequestData (
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
//     std::cout << "Slice Rep Input has extent: " << e[0] << " " << e[1]<< " " << e[2]<< " " << e[3]<< " " << e[4]<< " " << e[5] << std::endl;
//     std::cout << "Number Of Tuples " << input->GetPointData()->GetNumberOfTuples() << std::endl;

    vtkImageData *clone = vtkImageData::New();
    clone->ShallowCopy ( input );
//     clone->CopyInformation(input);
//     clone->GetExtent(e);
    Slice->SetInput ( clone );
    Slice->Update();
    this->SliceData->ShallowCopy ( Slice->GetOutput() );
    clone->Delete();
    input->GetBounds(SliceActor.bounds);
    this->DeliveryFilter->SetInput ( input );
  }
  else
  {
    this->DeliveryFilter->RemoveAllInputs();
  }

  return this->Superclass::RequestData ( request, inputVector, outputVector );
}


//----------------------------------------------------------------------------
vtkSmartPointer< vtkLookupTable > vtkSliceRepresentation::lut()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Build();
  return lut;
}

//----------------------------------------------------------------------------
void vtkSliceRepresentation::AddToView(vtkPVSliceView* view)
{
}

//----------------------------------------------------------------------------
bool vtkSliceRepresentation::AddToView(vtkView* view)
{
  //   qDebug() << "Add to View";
  vtkPVSliceView* rview = vtkPVSliceView::SafeDownCast(view);
  if (rview)
  {
    Slice->SetOutputDimensionality(2);
    Slice->SetResliceAxes(rview->GetSlicingMatrix());
    SliceProp->SetPickable(true);

    SliceActor.prop = SliceProp;
    SliceActor.mapper = Slice;

    SliceActor.lut = lut();
    Slice->SetLookupTable(SliceActor.lut);

    SliceActor.prop->SetOpacity(Opacity);
    SliceActor.prop->SetPosition(Position[0],Position[1],Position[2]);
    AddToView(rview);


    return true;
  }else
    return false;
}
//----------------------------------------------------------------------------
void vtkSliceRepresentation::SetVisibility(bool val)
{
  SliceProp->SetVisibility(val);
  vtkPVDataRepresentation::SetVisibility(val);
  SliceActor.visible = val;
}

//----------------------------------------------------------------------------
void vtkSliceRepresentation::SetColor(double color)
{
  Color = color;
//   std::cout<< "Changing Color " <<  color << std::endl;
  if (SliceActor.lut == NULL)
    return;

  double saturation = Color>=0?1.0:0;
  SliceActor.lut->SetSaturationRange(0.0, saturation);
  SliceActor.lut->SetHueRange(Color, Color);
  SliceActor.lut->Build();
}

//----------------------------------------------------------------------------
void vtkSliceRepresentation::SetOpacity(double value)
{
//   if (Opacity == value)
//     return;

  Opacity = value;
  if (SliceActor.prop)
    SliceActor.prop->SetOpacity(value);
  Modified();
}
