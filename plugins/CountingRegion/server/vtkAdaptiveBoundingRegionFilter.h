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
#ifndef VTKADAPTATIVEBOUNDINGREGIONFILTER_H
#define VTKADAPTATIVEBOUNDINGREGIONFILTER_H

#include <vtkPolyDataAlgorithm.h>


class VTK_IMAGING_EXPORT vtkAdaptiveBoundingRegionFilter
      : public vtkPolyDataAlgorithm
{
public:
  static vtkAdaptiveBoundingRegionFilter *New();
  vtkTypeMacro(vtkAdaptiveBoundingRegionFilter, vtkPolyDataAlgorithm);

  //! Inclusion Offset (Left, Top, Upper)
  vtkSetVector3Macro(InclusionOffset, double);
  vtkGetVector3Macro(InclusionOffset, double);
  //! Exclusion Offset (Right, Bottom, Lower)
  vtkSetVector3Macro(ExclusionOffset, double);
  vtkGetVector3Macro(ExclusionOffset, double);
  
  vtkGetMacro(TotalVolume, double);
  vtkGetMacro(TotalAdaptiveVolume, double);
  vtkGetMacro(InclusionVolume, double);
  vtkGetMacro(ExclusionVolume, double);
  vtkGetMacro(ExclusionAdaptiveVolume, double);

protected:
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

protected:
  vtkAdaptiveBoundingRegionFilter();
  virtual ~vtkAdaptiveBoundingRegionFilter();

private:
 // virtual vtkBoundingRegionFilter& operator=(const vtkBoundingRegionFilter& other); // Not implemented
 // virtual bool operator==(const vtkBoundingRegionFilter& other) const;// Not implemented
 double InclusionOffset[3];
 double ExclusionOffset[3];
 double TotalVolume;
 double TotalAdaptiveVolume;
 double InclusionVolume;
 double ExclusionVolume;
 double ExclusionAdaptiveVolume;
};

#endif // VTKADAPTATIVEBOUNDINGREGIONFILTER_H
