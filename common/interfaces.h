/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received  copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef INTERFACES_H
#define INTERFACES_H

#include "espinaTypes.h"

// Forward declaration
class pqOutputPort;
class pqPipelineSource;



//! Interface for Renderable objects:
//! This interface allows the views to display an object 
//! according to its properties
class IRenderable
{
public:
  enum RENDER_STYLE
  { VISIBLE   = 1
  , SELECTED  = 2
  , DISCARTED = 4
  };

public:
  IRenderable() : m_style(VISIBLE) {}
  
  IRenderable(pqPipelineSource *source, int portNumber)
      : m_style(VISIBLE)
      , m_source(source)
      , m_portNumber(portNumber)
  {}
  
  virtual bool visible() const
  {
    return m_style & VISIBLE;
  }
  
  virtual void setVisible(bool value)
  {
    m_style = RENDER_STYLE((m_style & !VISIBLE) | (value ? 1 : 0));
  }
  
  virtual RENDER_STYLE style() const
  {
    return m_style;
  }
  
  virtual pqOutputPort *outputPort() = 0;
  virtual pqPipelineSource *sourceData() = 0;
  virtual int portNumber() = 0;
  virtual void color(double *rgba) = 0;

protected:
  RENDER_STYLE m_style;
  pqPipelineSource *m_source;
  int m_portNumber;
};


#endif // INTERFACES_H
