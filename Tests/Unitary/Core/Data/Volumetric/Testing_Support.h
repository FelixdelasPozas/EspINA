/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_TESTING_SUPPORT_H
#define ESPINA_TESTING_SUPPORT_H

#include <vtkImplicitFunction.h>
#include <itkImageRegionIterator.h>

#include "Core/Utils/Bounds.h"
#include "Core/Analysis/Data/VolumetricDataUtils.h"

namespace EspINA
{

  class vtkNaiveFunction
  : public vtkImplicitFunction 
  {
  public:
    vtkTypeMacro(vtkNaiveFunction, vtkImplicitFunction);

    static vtkNaiveFunction *New();

    double EvaluateFunction(double x, double y, double z)   { return 0; }
    double EvaluateFunction(double xyz[3]) { return 0; }

    void EvaluateGradient(double x[3], double g[3]) { memcpy(g, x, 3*sizeof(double));}
  };

  template<typename T>
  class Testing_Support
  {
  public:
    static typename T::Pointer Create_Test_Image(const typename T::SizeType  size,
                                                 const typename T::ValueType value);

    static typename T::Pointer Create_Test_Image(const typename T::PointType   origin,
                                                 const typename T::IndexType   index,
                                                 const typename T::SizeType    size,
                                                 const typename T::SpacingType spacing,
                                                 const typename T::ValueType   value);

    static bool Test_Pixel_Values(const typename T::Pointer   image,
                                  const typename T::ValueType value,
                                  const Bounds& bounds = Bounds());
  };

#include "Testing_Support.txx"

} // namespace EspINA

#endif // ESPINA_TESTING_SUPPORT_H
