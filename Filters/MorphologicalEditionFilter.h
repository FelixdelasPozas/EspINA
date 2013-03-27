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

#include <Core/Model/Segmentation.h>

namespace EspINA
{

class MorphologicalEditionFilter
: public SegmentationFilter
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
                                      Arguments   args,
                                      FilterType  type);
  virtual ~MorphologicalEditionFilter();

  unsigned int radius() const {return m_params.radius();}
  void setRadius(int radius, bool ignoreUpdate = false)
  {m_params.setRadius(radius); m_paramModified = !ignoreUpdate;}

  virtual bool needUpdate(OutputId oId) const;

  virtual bool isOutputEmpty() { return m_isOutputEmpty; };

protected:
  /// Implements Filter Interface
  virtual bool fetchSnapshot(OutputId oId);

protected:
  Parameters             m_params;
  itkVolumeType::Pointer m_input;
  bool                   m_isOutputEmpty;
  bool                   m_paramModified;
};

} // namespace EspINA


#endif // MORPHOLOGICALEDITIONFILTER_H
