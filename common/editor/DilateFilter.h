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

#include <model/Segmentation.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkDilateObjectMorphologyImageFilter.h>

static const QString Dilate = "EditorToolBar::DilateFilter";

class DilateFilter
: public Filter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::DilateObjectMorphologyImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  class DilateArguments;

public:
  explicit DilateFilter(Segmentation *seg, unsigned int radius);
  explicit DilateFilter(Arguments args);
  virtual ~DilateFilter();

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
  DilateArguments *m_args;
  Segmentation  *m_input;
  EspinaVolume  *m_volume;

  FilterType::Pointer m_filter;
};

class DilateFilter::DilateArguments
: public Arguments
{
public:
  static const ModelItem::ArgumentId INPUT;
  static const ModelItem::ArgumentId RADIUS;

public:
  explicit DilateArguments(){}
  explicit DilateArguments(const Arguments args);

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

#endif // DILATEFILTER_H