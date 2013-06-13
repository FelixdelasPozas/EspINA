/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

// EspINA
#include "Core/EspinaTypes.h"
#include "vtkVoxelContour2D.h"

// VTK
#include <vtkObjectFactory.h> //for new() macro
#include <vtkImageData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkCleanPolyData.h>
#include <vtkSmartPointer.h>
#include <QDebug>

vtkStandardNewMacro(vtkVoxelContour2D);

//-----------------------------------------------------------------------------
vtkVoxelContour2D::vtkVoxelContour2D()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkVoxelContour2D::~vtkVoxelContour2D()
{
}

//-----------------------------------------------------------------------------
int vtkVoxelContour2D::RequestData(vtkInformation *request,
                                  vtkInformationVector **inputVector,
                                  vtkInformationVector *outputVector)
{
  // get the input
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input)
  {
    return 0;
  }

  // handle only AXIAL 2D images
  if (vtkImageData::SafeDownCast(input))
  {
    int *uExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    if (uExt[4] != uExt[5])
    {
      return 0;
    }
  }
  else
  {
    // input should be a vtkImageData
    Q_ASSERT(false);
  }

  // get output
  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
  {
    return 0;
  }

  vtkCellArray *newLines = vtkCellArray::New();
  newLines->Initialize();
  vtkPoints *newPts = vtkPoints::New();
  newPts->Initialize();

  vtkImageData *image = vtkImageData::SafeDownCast(input);
  int extent[6];
  image->GetExtent(extent);
  EspINA::Nm spacing[3];
  image->GetSpacing(spacing);

  // reslice can change the origin of the result to something different from (0,0,0)
  // depending on the reslice matrix direction.
  EspINA::Nm origin[3];
  image->GetOrigin(origin);

  // apparently all our slices are in the axial plane, go figure...
  if (extent[4] != extent[5])
    Q_ASSERT(false);

  unsigned char previousValue, *voxel;
  int x, y;
  vtkIdType cell[2];

  for (y = extent[2]; y <= extent[3]; ++y)
  {
    previousValue = EspINA::SEG_BG_VALUE;
    for (x = extent[0]; x <= extent[1]; ++x)
    {
      voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x, y, extent[4]));
      if (*voxel != previousValue)
      {
        cell[0] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
        cell[1] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y + 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
        newLines->InsertNextCell(2, cell);
      }
      previousValue = *voxel;
    }

    if (*voxel == EspINA::SEG_VOXEL_VALUE)
    {
      cell[0] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
      cell[1] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y + 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
      newLines->InsertNextCell(2, cell);
    }
  }

  for (x = extent[0]; x <= extent[1]; ++x)
  {
    previousValue = EspINA::SEG_BG_VALUE;
    for (y = extent[2]; y <= extent[3]; ++y)
    {
      voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x, y, extent[4]));
      if (*voxel != previousValue)
      {
        cell[0] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
        cell[1] = newPts->InsertNextPoint((((EspINA::Nm) x + 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4]*spacing[2] + origin[2]);
        newLines->InsertNextCell(2, cell);
      }
      previousValue = *voxel;
    }

    if (*voxel == EspINA::SEG_VOXEL_VALUE)
    {
      cell[0] = newPts->InsertNextPoint((((EspINA::Nm) x - 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4] + origin[2]);
      cell[1] = newPts->InsertNextPoint((((EspINA::Nm) x + 0.5) * spacing[0]) + origin[0], (((EspINA::Nm) y - 0.5) * spacing[1]) + origin[1], extent[4] + origin[2]);
      newLines->InsertNextCell(2, cell);
    }
  }

  vtkPolyData *temp = vtkPolyData::New();
  temp->SetPoints(newPts);
  temp->SetLines(newLines);
  newPts->Delete();
  newLines->Delete();

  vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
  cleaner->SetInput(temp);
  cleaner->PointMergingOn();
  cleaner->SetTolerance(0.0);
  cleaner->Update();

  temp->Delete();

  output->SetPoints(cleaner->GetOutput()->GetPoints());
  output->SetLines(cleaner->GetOutput()->GetLines());

  return 1;
}


//----------------------------------------------------------------------------
void vtkVoxelContour2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkPolyData* vtkVoxelContour2D::GetOutput()
{
  return this->GetOutput(0);
}

//----------------------------------------------------------------------------
vtkPolyData* vtkVoxelContour2D::GetOutput(int port)
{
  return vtkPolyData::SafeDownCast(this->GetOutputDataObject(port));
}

//----------------------------------------------------------------------------
void vtkVoxelContour2D::SetOutput(vtkDataObject* d)
{
  this->GetExecutive()->SetOutputData(0, d);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkVoxelContour2D::GetInput()
{
  return this->GetInput(0);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkVoxelContour2D::GetInput(int port)
{
  return this->GetExecutive()->GetInputData(port, 0);
}

//----------------------------------------------------------------------------
int vtkVoxelContour2D::ProcessRequest(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector)
{
  // Create an output object of the correct type.
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
  {
    return this->RequestDataObject(request, inputVector, outputVector);
  }
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    return this->RequestData(request, inputVector, outputVector);
  }

  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
  {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
  }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    return this->RequestInformation(request, inputVector, outputVector);
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkVoxelContour2D::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkVoxelContour2D::RequestDataObject(vtkInformation* vtkNotUsed(request),
                                         vtkInformationVector** vtkNotUsed(inputVector),
                                         vtkInformationVector* outputVector )
{
//RequestDataObject (RDO) is an earlier pipeline pass.
//During RDO, each filter is supposed to produce an empty data object of the proper type

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get( vtkDataObject::DATA_OBJECT() ) );


  if (!output)
  {
    output = vtkPolyData::New();
    outInfo->Set(vtkDataObject::DATA_OBJECT(), output);
    output->FastDelete();
    output->SetPipelineInformation(outInfo);
    this->GetOutputPortInformation(0)->Set(vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
  }

  return 1;
}


//----------------------------------------------------------------------------
int vtkVoxelContour2D::RequestInformation(vtkInformation* vtkNotUsed(request),
                                          vtkInformationVector** vtkNotUsed(inputVector),
                                          vtkInformationVector* vtkNotUsed(outputVector))
{
  // do nothing let subclasses handle it
  return 1;
}

//----------------------------------------------------------------------------
int vtkVoxelContour2D::RequestUpdateExtent(
                                          vtkInformation* vtkNotUsed(request),
    vtkInformationVector** inputVector,
    vtkInformationVector* vtkNotUsed(outputVector))
{
  int numInputPorts = this->GetNumberOfInputPorts();
  for (int i=0; i<numInputPorts; i++)
  {
    int numInputConnections = this->GetNumberOfInputConnections(i);
    for (int j=0; j<numInputConnections; j++)
    {
      vtkInformation* inputInfo = inputVector[i]->GetInformationObject(j);
      inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
    }
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkVoxelContour2D::SetInput(vtkDataObject* input)
{
  this->SetInput(0, input);
}

//----------------------------------------------------------------------------
void vtkVoxelContour2D::SetInput(int index, vtkDataObject* input)
{
  if(input)
  {
    this->SetInputConnection(index, input->GetProducerPort());
  }
  else
  {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(index, 0);
  }
}

//----------------------------------------------------------------------------
void vtkVoxelContour2D::AddInput(vtkDataObject* input)
{
  this->AddInput(0, input);
}

//----------------------------------------------------------------------------
void vtkVoxelContour2D::AddInput(int index, vtkDataObject* input)
{
  if(input)
  {
    this->AddInputConnection(index, input->GetProducerPort());
  }
}
