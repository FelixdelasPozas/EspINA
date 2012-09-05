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


#ifndef OPENINGFILTER_H
#define OPENINGFILTER_H

#include "MorphologicalEditionFilter.h"
#include "CODESettings.h"

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>

class OpeningFilter
: public MorphologicalEditionFilter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalOpeningImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  static const QString TYPE;

public:
  explicit OpeningFilter(NamedInputs inputs,
                         Arguments args);
  virtual ~OpeningFilter();


  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  virtual QWidget* createConfigurationWidget()
  { return new CODESettings(this, tr("Open")); }

protected:
  /// Implements Filter Interface
  void run();

private:
  FilterType::Pointer m_filter;
};


#endif // OPENINGFILTER_H