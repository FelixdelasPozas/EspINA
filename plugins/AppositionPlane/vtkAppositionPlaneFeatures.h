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


#ifndef VTKAPPOSITIONPLANEFEATURES_H
#define VTKAPPOSITIONPLANEFEATURES_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

// BTX
class vtkDoubleArray;
// ETX

class VTK_IMAGING_EXPORT vtkAppositionPlaneFeatures 
: public vtkPolyDataAlgorithm
{
public:
  static vtkAppositionPlaneFeatures *New();
  vtkTypeMacro(vtkAppositionPlaneFeatures, vtkPolyDataAlgorithm);
  
  vtkGetMacro(Area, double);
  vtkGetMacro(Perimeter, double);
  
protected:
  vtkAppositionPlaneFeatures();
  virtual ~vtkAppositionPlaneFeatures();
  
  double area(vtkPolyData *mesh);
  double perimeter(vtkPolyData *mesh);
  
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
  
private:
  vtkAppositionPlaneFeatures(const vtkPolyDataAlgorithm &);// Not implemented
//   virtual void operator=(const vtkPolyDataAlgorithm &);    // Not implemented
  
private:
  double Area;
  double Perimeter;
  
//BTX
private:
  vtkSmartPointer<vtkDoubleArray> _gauss_curves;
  vtkSmartPointer<vtkDoubleArray> _mean_Curves;
  vtkSmartPointer<vtkDoubleArray> _max_curves;
  vtkSmartPointer<vtkDoubleArray> _min_curves;
//ETX
};

#endif // VTKAPPOSITIONPLANEFEATURES_H
