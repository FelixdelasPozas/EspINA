/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef VTKCHANNELBORDERDETECTOR_H
#define VTKCHANNELBORDERDETECTOR_H

#include <vtkPolyDataAlgorithm.h>

#include <vtkCellArray.h>
#include <vtkSmartPointer.h>

class vtkImageData;

class  VTK_IMAGING_EXPORT vtkChannelBorderDetector
: public vtkPolyDataAlgorithm
{
  public:
  static vtkChannelBorderDetector *New();
  vtkTypeMacro(vtkChannelBorderDetector, vtkPolyDataAlgorithm);

  vtkGetMacro(TotalVolume, double);
  vtkGetMacro(TotalAdaptiveVolume, double);

protected:
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);

  void computeChannelMargins(vtkImageData *image);

protected:
  vtkChannelBorderDetector();
  virtual ~vtkChannelBorderDetector();

private:
 // virtual vtkBoundingRegionFilter& operator=(const vtkBoundingRegionFilter& other); // Not implemented
 // virtual bool operator==(const vtkBoundingRegionFilter& other) const;// Not implemented
 double TotalVolume;
 double TotalAdaptiveVolume;

 //BTX
 bool   m_init;
 vtkSmartPointer<vtkPoints>    borderVertices;
 vtkSmartPointer<vtkCellArray> faces;
 //ETX
};

#endif // VTKCHANNELBORDERDETECTOR_H
