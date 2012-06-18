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


#ifndef VTKFREEFORMSOURCE_H
#define VTKFREEFORMSOURCE_H

#include <vtkImageAlgorithm.h>
#include <vtkSmartPointer.h>

class vtkImageData;

class vtkFreeFormSource
: public vtkImageAlgorithm
{
public:
  static vtkFreeFormSource *New();
  vtkTypeMacro(vtkFreeFormSource, vtkImageAlgorithm);

//   vtkSetVector6Macro(Extent, int);
  vtkSetVector3Macro(Spacing, double);

  void Draw(int cx, int cy, int cz,
	    int r, int plane);
  void Draw(int disc[5]);

  void Erase(int cx, int cy, int cz,
	    int r, int plane);
  void Erase(int disc[5]);

protected:
  vtkFreeFormSource();
  virtual ~vtkFreeFormSource(){}

  virtual int RequestInformation(vtkInformation* request,
			        vtkInformationVector** inputVector,
			        vtkInformationVector* outputVector);
virtual int RequestData(vtkInformation* request,
		       vtkInformationVector** inputVector,
		       vtkInformationVector* outputVector);

private:
  int    Extent[6];
  int    DrawExtent[6];
  double Spacing[3];
  //BTX
  bool   m_init;
  vtkSmartPointer<vtkImageData> m_data;
  //ETX
};

#endif // VTKFREEFORMSOURCE_H
