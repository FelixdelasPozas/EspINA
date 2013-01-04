/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#ifndef CLOSINGFILTER_H
#define CLOSINGFILTER_H

#include "MorphologicalEditionFilter.h"

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>

namespace EspINA
{

class ClosingFilter
: public MorphologicalEditionFilter
{
  typedef itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalClosingImageFilter<itkVolumeType, itkVolumeType, StructuringElementType> BinaryClosingFilter;

public:
  explicit ClosingFilter(NamedInputs inputs,
                         Arguments   args,
                         FilterType  type);
  virtual ~ClosingFilter();

protected:
  /// Implements Filter Interface
  void run();

private:
  BinaryClosingFilter::Pointer m_filter;
};

} // namespace EspINA


#endif // CLOSINGFILTER_H
