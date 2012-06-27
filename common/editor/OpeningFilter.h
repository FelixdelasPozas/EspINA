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

class OpeningFilter
: public Filter
{
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalOpeningImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> FilterType;

public:
  static const QString TYPE;

  static const ModelItem::ArgumentId RADIUS;

  class OpeningArguments : public Arguments
  {
  public:
  public:
    explicit OpeningArguments(){}
    explicit OpeningArguments(const Arguments args) : Arguments(args){}

    void setRadius(unsigned int radius)
    {
      (*this)[RADIUS] = QString::number(radius);
    }
    unsigned int radius() const {return (*this)[RADIUS].toInt();}
  };

public:
  explicit OpeningFilter(NamedInputs inputs,
                         Arguments args);
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
  NamedInputs      m_inputs;
  OpeningArguments m_args;

  EspinaVolume    *m_input;
  EspinaVolume    *m_volume;

  FilterType::Pointer m_filter;
};


#endif // OPENINGFILTER_H