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


#ifndef MORPHOLOGICALEDITIONFILTER_H
#define MORPHOLOGICALEDITIONFILTER_H

#include <model/Segmentation.h>

class MorphologicalEditionFilter
: public Filter
{
public:

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
  explicit MorphologicalEditionFilter(NamedInputs inputs,
                                      Arguments args);
  virtual ~MorphologicalEditionFilter();

  /// Implements Filter Interface
  virtual bool needUpdate() const;
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();

  virtual QWidget* createConfigurationWidget();

  unsigned int radius() const {return m_params.radius();}
  void setRadius(int radius) {m_params.setRadius(radius); run();}

protected:
  Parameters       m_params;
  EspinaVolume    *m_input;
  EspinaVolume    *m_volume;
  EspinaVolumeReader::Pointer m_cachedFilter;
};


#endif // MORPHOLOGICALEDITIONFILTER_H