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
#ifndef VTKRECTANGULARBOUNDINGREGIONFILTER_H
#define VTKRECTANGULARBOUNDINGREGIONFILTER_H

#include <vtkPolyDataAlgorithm.h>


class VTK_IMAGING_EXPORT vtkRectangularBoundingRegionFilter
      : public vtkPolyDataAlgorithm
{
public:
  static vtkRectangularBoundingRegionFilter *New();
  vtkTypeMacro(vtkRectangularBoundingRegionFilter, vtkPolyDataAlgorithm);

  vtkSetVector6Macro(Margin, double);//in nm

  /// Inclusion Coordinates (Left, Top, Upper)
  vtkSetVector3Macro(InclusionOffset, double);//in nm
  vtkGetVector3Macro(InclusionOffset, double);//in nm
  /// Exclusion Coordinates (Right, Bottom, Lower)
  vtkSetVector3Macro(ExclusionOffset, double);//in nm
  vtkGetVector3Macro(ExclusionOffset, double);//in nm

  vtkGetMacro(TotalVolume,     double); //in nm3
  vtkGetMacro(InclusionVolume, double); //in nm3
  vtkGetMacro(ExclusionVolume, double); //in nm3

protected:
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

protected:
  vtkRectangularBoundingRegionFilter();
  virtual ~vtkRectangularBoundingRegionFilter();

private:
  double left()  {return Margin[0]+InclusionOffset[0];}
  double top()   {return Margin[2]+InclusionOffset[1];}
  double upper() {return Margin[4]+InclusionOffset[2];}
  double right() {return Margin[1]-ExclusionOffset[0];}
  double bottom(){return Margin[3]-ExclusionOffset[1];}
  double lower() {return Margin[5]-ExclusionOffset[2];}

private:
//  virtual vtkRectangularBoundingRegionFilter& operator=(const vtkRectangularBoundingRegionFilter& other); // Not implemented
//  virtual bool operator==(const vtkRectangularBoundingRegionFilter& other) const;// Not implemented

 double Margin[6];
 double InclusionOffset[3];
 double ExclusionOffset[3];
 double TotalVolume;
 double InclusionVolume;
 double ExclusionVolume;
};

#endif // VTKRECTANGULARBOUNDINGREGIONFILTER_H
