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


#include "vtkFreeFormSource.h"
#include <vtkSliceView.h>

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <cassert>

vtkStandardNewMacro(vtkFreeFormSource);

//-----------------------------------------------------------------------------
void printExtent(int extent[6])
{
    std::cout <<
    extent[0] <<  " " << extent[1] <<  " " <<
    extent[2] <<  " " << extent[3] <<  " " <<
    extent[4] <<  " " << extent[5] <<  " " <<
    std::endl;
}

//-----------------------------------------------------------------------------
vtkFreeFormSource::vtkFreeFormSource()
: m_init(false)
{
  memset(Extent, 0, 6*sizeof(int));
  memset(DrawExtent, 0, 6*sizeof(int));
  Spacing[0] = Spacing[1] = Spacing[2] = 1;

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
bool drawPixel(int x, int y, int z,
	      int cx, int cy, int cz,
	      int r, int plane,
	      int extent[6])
{
  if (plane == 2)
  {
    double r2 = pow(x-cx+extent[0], 2) + pow(y-cy+extent[2], 2);
    return r2 <= r*r && z == cz-extent[4];
  }else if (plane == 1)
  {
    double r2 = pow(x-cx+extent[0], 2) + pow(z-cz+extent[4], 2);
    return r2 <= r*r && y == cy-extent[2];
  }else if (plane == 0)
  {
    double r2 = pow(y-cy+extent[2], 2) + pow(z-cz+extent[4], 2);
    return r2 <= r*r && x == cx-extent[0];
  }
  return false;
}

//-----------------------------------------------------------------------------
void vtkFreeFormSource::Draw(int cx, int cy, int cz, int r, int plane)
{
  if (0 <= plane && plane <=2 && r >= 0)
  {
//     std::cout << "Drawing" << std::endl;
//     std::cout << "\t" << cx << " " << cy << " " << cz << " " << r << std::endl;

    bool expandX = vtkSliceView::AXIAL == plane
                || vtkSliceView::CORONAL == plane;
    bool expandY = vtkSliceView::AXIAL == plane
                || vtkSliceView::SAGITTAL == plane;
    bool expandZ = vtkSliceView::SAGITTAL == plane
                || vtkSliceView::CORONAL == plane;
	
    if (!m_init)
    {
      Extent[0] = Extent[1] = cx;
      Extent[2] = Extent[3] = cy;
      Extent[4] = Extent[5] = cz;
      m_init = true;
    }

    int dim[3];
    int prevExtent[6];
    memcpy(prevExtent, Extent, 6*sizeof(int));


    Extent[0] = std::min(Extent[0], expandX?cx - r:cx);
    Extent[1] = std::max(Extent[1], expandX?cx + r:cx);
    Extent[2] = std::min(Extent[2], expandY?cy - r:cy);
    Extent[3] = std::max(Extent[3], expandY?cy + r:cy);
    Extent[4] = std::min(Extent[4], expandZ?cz - r:cz);
    Extent[5] = std::max(Extent[5], expandZ?cz + r:cz);

    if (memcmp(prevExtent, Extent, 6*sizeof(int)) == 0)
    {
      int minX = (expandX?cx - r:cx)-Extent[0];
      int maxX = (expandX?cx + r:cx)-Extent[0];
      int minY = (expandY?cy - r:cy)-Extent[2];
      int maxY = (expandY?cy + r:cy)-Extent[2];
      int minZ = (expandZ?cz - r:cz)-Extent[4];
      int maxZ = (expandZ?cz + r:cz)-Extent[4];

      m_data->GetDimensions(dim);

      //NOTE: Try using vtkImageIterator 
      unsigned char *outputPtr = static_cast<unsigned char *>(m_data->GetScalarPointer());

      for (int z = minZ; z <= maxZ; z++)
      {
	unsigned long long zOffset = z * dim[0] * dim[1];
	for (int y = minY; y <= maxY; y++)
	{
	  unsigned long long yOffset = y * dim[0];
	  for (int x = minX; x <= maxX; x++)
	  {
	    unsigned long long offset = x + yOffset + zOffset;
	    if (drawPixel(x,y,z,cx,cy,cz,r,plane,Extent))
	      outputPtr[offset] = 255;
	  }
	}
      }
    }else
    {
      vtkSmartPointer<vtkImageData> img
      = vtkSmartPointer<vtkImageData>::New();

      //     std::cout << "New Extent\n";
      //       printExtent(Extent);
      img->SetExtent(Extent);
      img->SetScalarTypeToUnsignedChar();
      img->SetNumberOfScalarComponents(1);
      img->AllocateScalars();

      unsigned char *prevOutputPtr;
      int prevDim[3];
      if (!m_data.GetPointer())
      {
	prevExtent[0] = prevExtent[2] = prevExtent[4] = 0;
	prevExtent[1] = prevExtent[3] = prevExtent[5] = -1;
      } else
      {
	prevOutputPtr = static_cast<unsigned char *>(m_data->GetScalarPointer());
	m_data->GetDimensions(prevDim);
      }

      int minX = 0;
      int maxX = Extent[1]-Extent[0];
      int minY = 0;
      int maxY = Extent[3]-Extent[2];
      int minZ = 0;
      int maxZ = Extent[5]-Extent[4];

      unsigned char *outputPtr = static_cast<unsigned char *>(img->GetScalarPointer());
      img->GetDimensions(dim);

      for (int z = minZ; z <= maxZ; z++)
      {
	unsigned long long zOffset = z * dim[0] * dim[1];
	for (int y = minY; y <= maxY; y++)
	{
	  unsigned long long yOffset = y * dim[0];
	  for (int x = minX; x <= maxX; x++)
	  {
	    unsigned long long offset = x + yOffset + zOffset;
	    double prevValue = 0;
	    if (prevExtent[0] <= x + Extent[0] && x + Extent[0] <= prevExtent[1]
	      && prevExtent[2] <= y + Extent[2] && y + Extent[2] <= prevExtent[3]
	      && prevExtent[4] <= z + Extent[4] && z + Extent[4] <= prevExtent[5])
	    {
	      prevValue = *prevOutputPtr;
	      prevOutputPtr++;
	    }
	    outputPtr[offset] = drawPixel(x,y,z,cx,cy,cz,r,plane,Extent)?255:prevValue;
	  }
	}
      }
      m_data = img;
    }

	Modified();
  }
}

//-----------------------------------------------------------------------------
void vtkFreeFormSource::Draw(int disc[5])
{
  Draw(disc[0], disc[1], disc[2], disc[3], disc[4]);
}

//-----------------------------------------------------------------------------
void vtkFreeFormSource::Erase(int cx, int cy, int cz, int r, int plane)
{
  if (0 <= plane && plane <=2 && r > 0
    && m_data.GetPointer()
    && Extent[0] <= cx + r && cx - r <= Extent[1]
    && Extent[2] <= cy + r && cy - r <= Extent[3]
    && Extent[4] <= cz + r && cz - r <= Extent[5]
  )
  {
    DrawExtent[0] = cx - r;
    DrawExtent[1] = cx + r;
    DrawExtent[2] = cy - r;
    DrawExtent[3] = cy + r;
    DrawExtent[4] = cz - r;
    DrawExtent[5] = cz + r;
//     std::cout << "Erasing" << std::endl;
//     std::cout << "\t" << cx << " " << cy << " " << cz << " " << r << std::endl;
    bool expandX = vtkSliceView::AXIAL == plane
                || vtkSliceView::CORONAL == plane;
    bool expandY = vtkSliceView::AXIAL == plane
                || vtkSliceView::SAGITTAL == plane;
    bool expandZ = vtkSliceView::SAGITTAL == plane
                || vtkSliceView::CORONAL == plane;

    int minX = std::max(Extent[0],(expandX?cx - r:cx))-Extent[0];
    int maxX = std::min(Extent[1],(expandX?cx + r:cx))-Extent[0];
    int minY = std::max(Extent[2],(expandY?cy - r:cy))-Extent[2];
    int maxY = std::min(Extent[3],(expandY?cy + r:cy))-Extent[2];
    int minZ = std::max(Extent[4],(expandZ?cz - r:cz))-Extent[4];
    int maxZ = std::min(Extent[5],(expandZ?cz + r:cz))-Extent[4];

    int dim[3];
    m_data->GetDimensions(dim);

    //NOTE: Try using vtkImageIterator
    unsigned char *outputPtr = static_cast<unsigned char *>(m_data->GetScalarPointer());

    for (int z = minZ; z <= maxZ; z++)
    {
      unsigned long long zOffset = z * dim[0] * dim[1];
      for (int y = minY; y <= maxY; y++)
      {
	unsigned long long yOffset = y * dim[0];
	for (int x = minX; x <= maxX; x++)
	{
	  unsigned long long offset = x + yOffset + zOffset;
	  if (drawPixel(x,y,z,cx,cy,cz,r,plane,Extent))
	    outputPtr[offset] = 0;
	}
      }
    }

    Modified();
  }
}

//-----------------------------------------------------------------------------
void vtkFreeFormSource::Erase(int disc[5])
{
  Erase(disc[0], disc[1], disc[2], disc[3], disc[4]);
}
//-----------------------------------------------------------------------------
int vtkFreeFormSource::RequestInformation(vtkInformation* request,
					vtkInformationVector** inputVector,
					vtkInformationVector* outputVector)
{
  //Get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int res = vtkImageAlgorithm::RequestInformation(request, inputVector, outputVector);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), Extent, 6);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), DrawExtent, 6);
  outInfo->Set(vtkDataObject::SPACING(), Spacing, 3);

  return res;
}

//-----------------------------------------------------------------------------
int vtkFreeFormSource::RequestData(vtkInformation* request,
				  vtkInformationVector** inputVector,
				  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()) );

  output->ShallowCopy(m_data);
  output->CopyInformation(m_data);
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());
  output->SetSpacing(Spacing);
  outInfo->Set(vtkDataObject::SPACING(), Spacing, 3);

  return 1;
}

