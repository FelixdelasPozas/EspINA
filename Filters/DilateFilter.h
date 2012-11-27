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


#ifndef DILATEFILTER_H
#define DILATEFILTER_H

#include "MorphologicalEditionFilter.h"

#include <Core/Model/Segmentation.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkDilateObjectMorphologyImageFilter.h>



class DilateFilter
: public MorphologicalEditionFilter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::DilateObjectMorphologyImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  static const QString TYPE;

public:
  explicit DilateFilter(NamedInputs inputs,
                         Arguments args);
  virtual ~DilateFilter();

  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

protected:
  /// Implements Filter Interface
  void run();

private:
  FilterType::Pointer m_filter;
};


#endif // DILATEFILTER_H
