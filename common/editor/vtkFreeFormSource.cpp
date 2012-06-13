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
#include <vtkPVSliceView.h>

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
  Spacing[0] = Spacing[1] = Spacing[2] = 1;

  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
void vtkFreeFormSource::Draw(int cx, int cy, int cz, int r, int plane)
{
  if (0 <= plane && plane <=2 && r > 0)
  {
//     std::cout << "Drawing" << std::endl;
//     std::cout << "\t" << cx << " " << cy << " " << cz << " " << r << std::endl;

    bool expandX = vtkPVSliceView::AXIAL == plane
                || vtkPVSliceView::CORONAL == plane;
    bool expandY = vtkPVSliceView::AXIAL == plane
                || vtkPVSliceView::SAGITTAL == plane;
    bool expandZ = vtkPVSliceView::SAGITTAL == plane
                || vtkPVSliceView::CORONAL == plane;
	
    if (!m_init)
    {
      Extent[0] = Extent[1] = cx;
      Extent[2] = Extent[3] = cy;
      Extent[4] = Extent[5] = cz;
      m_init = true;
    }

    Extent[0] = std::min(Extent[0], expandX?cx - r:cx);
    Extent[1] = std::max(Extent[1], expandX?cx + r:cx);
    Extent[2] = std::min(Extent[2], expandY?cy - r:cy);
    Extent[3] = std::max(Extent[3], expandY?cy + r:cy);
    Extent[4] = std::min(Extent[4], expandZ?cz - r:cz);
    Extent[5] = std::max(Extent[5], expandZ?cz + r:cz);

    vtkSmartPointer<vtkImageData> img
    = vtkSmartPointer<vtkImageData>::New();

//     std::cout << "New Extent\n";
//       printExtent(Extent);
    img->SetExtent(Extent);
    img->SetScalarTypeToUnsignedChar();
    img->SetNumberOfScalarComponents(1);
    img->AllocateScalars();

    int lastExtent[6] = {0, -1, 0, -1, 0, -1};
    if (m_data.GetPointer())
      m_data->GetExtent(lastExtent);

    for (int x = Extent[0]; x <= Extent[1]; x++)
      for (int y = Extent[2]; y <= Extent[3]; y++)
	for (int z = Extent[4]; z <= Extent[5]; z++)
	{
	  double r2 = pow(x-cx, 2) + pow(y-cy, 2);
	  double value = r2 <= r*r && z == cz?255:0;
	  double prevValue = 0;
	  if (lastExtent[0] <= x && x <= lastExtent[1]
	    && lastExtent[2] <= y && y <= lastExtent[3]
	    && lastExtent[4] <= z && z <= lastExtent[5])
	  {
	    prevValue = m_data->GetScalarComponentAsDouble(x, y, z, 0);
	  }
	  value = value + prevValue;

	  img->SetScalarComponentFromDouble(x,y,z,0,value);
	}

	m_data = img;

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
  if (0 <= plane && plane <=2 && r > 0)
  {
//     std::cout << "Erasing" << std::endl;
//     std::cout << "\t" << cx << " " << cy << " " << cz << " " << r << std::endl;

//     bool expandX = vtkPVSliceView::AXIAL == plane
//                 || vtkPVSliceView::CORONAL == plane;
//     bool expandY = vtkPVSliceView::AXIAL == plane
//                 || vtkPVSliceView::SAGITTAL == plane;
//     bool expandZ = vtkPVSliceView::SAGITTAL == plane
//                 || vtkPVSliceView::CORONAL == plane;
// 	
//     if (!m_init)
//     {
//       Extent[0] = Extent[1] = cx;
//       Extent[2] = Extent[3] = cy;
//       Extent[4] = Extent[5] = cz;
//       m_init = true;
//     }
// 
//     Extent[0] = std::min(Extent[0], expandX?cx - r:cx);
//     Extent[1] = std::max(Extent[1], expandX?cx + r:cx);
//     Extent[2] = std::min(Extent[2], expandY?cy - r:cy);
//     Extent[3] = std::max(Extent[3], expandY?cy + r:cy);
//     Extent[4] = std::min(Extent[4], expandZ?cz - r:cz);
//     Extent[5] = std::max(Extent[5], expandZ?cz + r:cz);

    assert(m_data.GetPointer());

    for (int x = Extent[0]; x <= Extent[1]; x++)
      for (int y = Extent[2]; y <= Extent[3]; y++)
	for (int z = Extent[4]; z <= Extent[5]; z++)
	{
	  double r2 = pow(x-cx, 2) + pow(y-cy, 2);
	  if (r2 <= r*r && z == cz)
	    m_data->SetScalarComponentFromDouble(x, y, z, 0, 0.0);
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
int vtkFreeFormSource::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  //Get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int res = vtkImageAlgorithm::RequestInformation(request, inputVector, outputVector);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), Extent, 6);
  outInfo->Set(vtkDataObject::SPACING(), Spacing, 3);

  return res;
}

//-----------------------------------------------------------------------------
int vtkFreeFormSource::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
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

