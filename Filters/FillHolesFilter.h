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


#ifndef FILLHOLESFILTER_H
#define FILLHOLESFILTER_H

#include <Core/Model/Filter.h>

#include <itkBinaryFillholeImageFilter.h>

namespace EspINA
{
class FillHolesFilter
: public SegmentationFilter
{
  typedef itk::BinaryFillholeImageFilter<itkVolumeType> BinaryFillholeFilter;

public:
  static const QString INPUTLINK;

public:
  explicit FillHolesFilter(NamedInputs inputs,
                           Arguments   args,
                           FilterType  type);
  virtual ~FillHolesFilter();

  /// Implements Filter Interface
  virtual bool needUpdate() const;

protected:
  virtual void run();

private:
  BinaryFillholeFilter::Pointer m_filter;
};

} // namespace EspINA

#endif // FILLHOLESFILTER_H
