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

#include "Filters/BasicSegmentationFilter.h"

namespace EspINA
{

class MorphologicalEditionFilter
: public BasicSegmentationFilter
{
public:

  static const ModelItem::ArgumentId RADIUS;
  static const QString INPUTLINK;

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
  {m_params.setRadius(radius); m_ignoreCurrentOutputs = !ignoreUpdate;}

  virtual bool isOutputEmpty() { return m_isOutputEmpty; };

protected:
  virtual bool ignoreCurrentOutputs() const
  {  return m_ignoreCurrentOutputs; }

  virtual bool needUpdate(FilterOutputId oId) const;

  virtual void run()
  { run(0); }

  virtual void run(FilterOutputId oId) = 0;


protected:
  bool                       m_ignoreCurrentOutputs;
  bool                       m_isOutputEmpty;
  Parameters                 m_params;
};

} // namespace EspINA


#endif // MORPHOLOGICALEDITIONFILTER_H
