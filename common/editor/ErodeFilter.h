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


#ifndef ERODEFILTER_H
#define ERODEFILTER_H

#include <model/Segmentation.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkErodeObjectMorphologyImageFilter.h>

class ErodeFilter
: public Filter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::ErodeObjectMorphologyImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  static const QString TYPE;

  static const ModelItem::ArgumentId RADIUS;

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args) : m_args(args){}

    void setRadius(unsigned int radius)
    {
      m_args[RADIUS] = QString::number(radius);
    }
    unsigned int radius() const {return m_args[RADIUS].toInt();}
  private:
    Arguments &m_args;
  };

public:
  explicit ErodeFilter(NamedInputs inputs,
                         Arguments args);
  virtual ~ErodeFilter();

  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  /// Implements Filter Interface
  void run();
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();

private:
  Parameters     m_params;
  EspinaVolume  *m_input;
  EspinaVolume  *m_volume;
  EspinaVolumeReader::Pointer m_cachedFilter;

  FilterType::Pointer m_filter;
};


#endif // ERODEFILTER_H