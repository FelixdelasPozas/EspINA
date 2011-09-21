/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef VTKSEGMENTATIONSELECTIONFILTER_H
#define VTKSEGMENTATIONSELECTIONFILTER_H

#include <vtkImageAlgorithm.h>

class VTK_IMAGING_EXPORT vtkSegmentationSelectionFilter :
public vtkImageAlgorithm
{
public:
  static vtkSegmentationSelectionFilter *New();
  vtkTypeMacro(vtkSegmentationSelectionFilter, vtkImageAlgorithm);
  
  
  vtkSetVector3Macro(CheckPixel,int);
  vtkGetMacro(PixelValue,int);
  
protected:
  vtkSegmentationSelectionFilter();
  virtual ~vtkSegmentationSelectionFilter(){}

  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
  
private:
  vtkSegmentationSelectionFilter(const vtkImageAlgorithm&);//Not implemented
  void operator=(const vtkImageAlgorithm&);//Not implemented
  
  int CheckPixel[3];
  int PixelValue;
};

#endif // VTKSEGMENTATIONSELECTIONFILTER_H
