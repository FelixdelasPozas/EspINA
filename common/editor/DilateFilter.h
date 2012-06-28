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


class DilateFilter
: public Filter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::DilateObjectMorphologyImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  static const QString TYPE;

  static const ModelItem::ArgumentId RADIUS;

  class DilateArguments : public Arguments
  {
  public:
  public:
    explicit DilateArguments(){}
    explicit DilateArguments(const Arguments args) : Arguments(args){}

    void setRadius(unsigned int radius)
    {
      (*this)[RADIUS] = QString::number(radius);
    }
    unsigned int radius() const {return (*this)[RADIUS].toInt();}
  };

public:
  explicit DilateFilter(NamedInputs inputs,
                         Arguments args);
  virtual ~DilateFilter();

  void run();

  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();

  virtual QWidget* createConfigurationWidget();

private:
  NamedInputs     m_inputs;
  DilateArguments m_args;

  EspinaVolume   *m_input;
  EspinaVolume   *m_volume;
  EspinaVolumeReader::Pointer m_cachedFilter;

  FilterType::Pointer m_filter;
};


#endif // DILATEFILTER_H