/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 F�lix de las Pozas �lvarez <felixdelaspozas@gmail.com>

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
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>

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
: m_minSpacing(0.0)
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

  // handle only planar images
  if (vtkImageData::SafeDownCast(input))
  {
    int *uExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
    if ((uExt[0] != uExt[1]) && (uExt[2] != uExt[3]) && (uExt[4] != uExt[5]))
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

  EspINA::Plane plane;
  Q_ASSERT((extent[0] == extent[1]) || (extent[2] == extent[3]) || (extent[4] == extent[5]));
  if (extent[0] == extent[1])
    plane = EspINA::Plane::YZ;
  else
    if (extent[2] == extent[3])
      plane = EspINA::Plane::XZ;
    else
      if (extent[4] == extent[5])
        plane = EspINA::Plane::XY;

  EspINA::Nm spacing[3];
  image->GetSpacing(spacing);
  m_minSpacing = std::min(spacing[0], std::min(spacing[1], spacing[2]));
  Q_ASSERT(m_minSpacing != 0);

  EspINA::Nm origin[3];
  image->GetOrigin(origin);

  unsigned char previousValue;
  unsigned char *voxel = NULL;
  int x, y, z;
  double dx,dy,dz;
  vtkIdType cell[2];

  switch (plane)
  {
    case EspINA::Plane::XY:
    {
      dz = static_cast<double>(extent[5]);
      for (y = extent[2]; y <= extent[3]; ++y)
      {
        previousValue = EspINA::SEG_BG_VALUE;
        for (x = extent[0]; x <= extent[1]; ++x)
        {
          voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x, y, extent[4]));
          if (*voxel != previousValue)
          {
            dx = static_cast<double>(x);
            dy = static_cast<double>(y);
            cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dx = static_cast<double>(x);
          dy = static_cast<double>(y);
          cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
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
            dx = static_cast<double>(x);
            dy = static_cast<double>(y);
            cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dx = static_cast<double>(x);
          dy = static_cast<double>(y);
          cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5 ) * spacing[2]) + origin[2]);
          newLines->InsertNextCell(2, cell);
        }
      }
    }
      break;
    case EspINA::Plane::XZ:
    {
      dy = static_cast<double>(extent[2]);
      for (z = extent[4]; z <= extent[5]; ++z)
      {
        previousValue = EspINA::SEG_BG_VALUE;
        for (x = extent[0]; x <= extent[1]; ++x)
        {
          voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x, extent[2], z));
          if (*voxel != previousValue)
          {
            dx = static_cast<double>(x);
            dz = static_cast<double>(z);
            cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz + 0.5) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dx = static_cast<double>(x);
          dz = static_cast<double>(z);
          cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz + 0.5) * spacing[2]) + origin[2]);
          newLines->InsertNextCell(2, cell);
        }
      }

      for (x = extent[0]; x <= extent[1]; ++x)
      {
        previousValue = EspINA::SEG_BG_VALUE;
        for (z = extent[4]; z <= extent[5]; ++z)
        {
          voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(x, extent[2], z));
          if (*voxel != previousValue)
          {
            dx = static_cast<double>(x);
            dz = static_cast<double>(z);
            cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dx = static_cast<double>(x);
          dz = static_cast<double>(z);
          cell[0] = newPts->InsertNextPoint(((dx - 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          newLines->InsertNextCell(2, cell);
        }
      }

    }
      break;
    case EspINA::Plane::YZ:
    {
      dx = static_cast<double>(extent[0]);
      for (z = extent[4]; z <= extent[5]; ++z)
      {
        previousValue = EspINA::SEG_BG_VALUE;
        for (y = extent[2]; y <= extent[3]; ++y)
        {
          voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(extent[0], y, z));
          if (*voxel != previousValue)
          {
            dy = static_cast<double>(y);
            dz = static_cast<double>(z);
            cell[0] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz + 0.5) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dy = static_cast<double>(y);
          dz = static_cast<double>(z);
          cell[0] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz + 0.5) * spacing[2]) + origin[2]);
          newLines->InsertNextCell(2, cell);
        }
      }

      for (y = extent[2]; y <= extent[3]; ++y)
      {
        previousValue = EspINA::SEG_BG_VALUE;
        for (z = extent[4]; z <= extent[5]; ++z)
        {
          voxel = reinterpret_cast<unsigned char *>(image->GetScalarPointer(extent[0], y, z));
          if (*voxel != previousValue)
          {
            dy = static_cast<double>(y);
            dz = static_cast<double>(z);
            cell[0] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
            newLines->InsertNextCell(2, cell);
          }
          previousValue = *voxel;
        }

        if (*voxel == EspINA::SEG_VOXEL_VALUE)
        {
          dy = static_cast<double>(y);
          dz = static_cast<double>(z);
          cell[0] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy - 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          cell[1] = newPts->InsertNextPoint(((dx + 0.5) * spacing[0]) + origin[0], ((dy + 0.5) * spacing[1]) + origin[1], ((dz - 0.5) * spacing[2]) + origin[2]);
          newLines->InsertNextCell(2, cell);
        }
      }
    }
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  vtkPolyData *temp = vtkPolyData::New();
  temp->SetPoints(newPts);
  temp->SetLines(newLines);
  newPts->Delete();
  newLines->Delete();

  vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
  cleaner->SetInputData(temp);
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
int vtkVoxelContour2D::RequestUpdateExtent(vtkInformation* vtkNotUsed(request),
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
    this->SetInputData(index, input);
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
    this->AddInputData(index, input);
  }
}

//----------------------------------------------------------------------------
double vtkVoxelContour2D::getMinimumSpacing() const
{
  return m_minSpacing;
}

