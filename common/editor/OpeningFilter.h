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

#include <model/Segmentation.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>

static const QString Opening = "EditorToolBar::OpeningFilter";

class OpeningFilter
: public Filter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalOpeningImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  class OpeningArguments;

public:
  explicit OpeningFilter(Segmentation *seg, unsigned int radius);
  explicit OpeningFilter(Arguments args);
  virtual ~OpeningFilter();

  void run();

  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual QWidget* createConfigurationWidget();

private:
  OpeningArguments *m_args;
  Segmentation  *m_input;
  EspinaVolume  *m_volume;

  FilterType::Pointer m_filter;
};

class OpeningFilter::OpeningArguments
: public Arguments
{
public:
  static const ModelItem::ArgumentId INPUT;
  static const ModelItem::ArgumentId RADIUS;

public:
  explicit OpeningArguments(){}
  explicit OpeningArguments(const Arguments args);

  void setInput(Segmentation * seg)
  {
    (*this)[INPUT] = seg->id();
  }

  void setRadius(unsigned int radius)
  {
    m_radius = radius;
    (*this)[RADIUS] = QString::number(radius);
  }
  unsigned int radius() const {return m_radius;}

private:
  unsigned int m_radius;
};

#endif // OPENINGFILTER_H