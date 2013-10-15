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


#include "vtkTube.h"
#include <vtkObjectFactory.h>
#include <vtkMath.h>

vtkStandardNewMacro(vtkTube);

vtkTube::vtkTube()
: BaseRadius(0.5)
, TopRadius (0.5)
{
  this->BaseCenter[0] = this->BaseCenter[1] = this->BaseCenter[2] = 0.0;
  this->TopCenter[0] = this->TopCenter[1] = 0.0;
  this->TopCenter[2] = 1.0;
}


void vtkTube::EvaluateGradient(double xyz[3], double g[3])
{
  memset(g, 0, 3*sizeof(double));
}

double vtkTube::EvaluateFunction(double xyz[3])
{
  double u[3], v[3];
  for(int i=0; i<3; i++)
  {
    u[i] = xyz[i] - this->BaseCenter[i];
    v[i] = this->TopCenter[i] - this->BaseCenter[i];
  }

  double rr = this->TopRadius - this->BaseRadius; // Radius difference
  double dv = vtkMath::Norm(v); // v's length
  double dp = vtkMath::Dot(u, v) / dv; // u projection's length
  if (dp < 0 || dp > dv)
    return 1; // Outside
  
  double a = dp / dv; //Alpha
  
  double c[3]; // Sphere's centered on u's projection
  for(int i=0; i<3; i++)
    c[i] = v[i]*a + this->BaseCenter[i];

  double r = this->BaseRadius + rr*a;
  double x = xyz[0] - c[0];
  double y = xyz[1] - c[1];
  double z = xyz[2] - c[2];
  
  return (x*x + y*y + z*z - r*r);
}

