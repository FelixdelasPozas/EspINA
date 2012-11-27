/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef VTKTUBE_H
#define VTKTUBE_H

#include <vtkImplicitFunction.h>


class vtkTube
: public vtkImplicitFunction
{
public:
  vtkTypeMacro(vtkTube, vtkImplicitFunction);

  // Description
  // Construct truncated cone with both radius of 0.5.
  static vtkTube *New();

  virtual void EvaluateGradient(double xyz[3], double g[3]);
  virtual double EvaluateFunction(double xyz[3]);
  double EvaluateFunction(double x, double y, double z)
  {return this->vtkImplicitFunction::EvaluateFunction(x, y, z);}

  vtkSetVector3Macro(BaseCenter,double);
  vtkGetVectorMacro(BaseCenter,double,3);

  vtkSetMacro(BaseRadius, double);
  vtkGetMacro(BaseRadius, double);

  vtkSetVector3Macro(TopCenter,double);
  vtkGetVectorMacro(TopCenter,double,3);

  vtkSetMacro(TopRadius, double);
  vtkGetMacro(TopRadius, double);

protected:
  vtkTube();
  ~vtkTube(){}

  double BaseCenter[3];
  double BaseRadius;
  double TopCenter[3];
  double TopRadius;

private:
  vtkTube(const vtkTube&);
  void operator=(const vtkTube&);
};

#endif // vtkTube_H
