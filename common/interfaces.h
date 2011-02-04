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

class ISelectableObject
{
public:
  ISelectableObject(){}
  virtual ~ISelectableObject(){}
};

//! Tuple containing the selected object and its selected coordinate
struct Selection 
{
  ImagePixel coord;
  ISelectableObject *object;
};

//! Interface to handle selections
class ISelectionHandler
{
  
public:
  ISelectionHandler(){};
  virtual ~ISelectionHandler() = 0;

  //! Handles @sel
  virtual void handle(const Selection sel) = 0;
  virtual void abortSelection() = 0;
};

class IRenderable
{
public:
  enum RENDER_STYLE 
  { VISIBLE   = 2^0
  , SELECTED  = 2^1
  , DISCARTED = 2^2
  };
  
// protected:
//   enum RENDER_MASK 
//   { isVISIBLE   = 1
//   , isSELECTED  = 2^0
//   , isDISCARTED = 2^1
//   };
public:
  IRenderable() : m_style(VISIBLE) {}
  IRenderable(pqPipelineSource *source, int portNumber) 
  : m_style(VISIBLE) 
  , m_source(source)
  , m_portNumber(portNumber)
  {}
  virtual bool visible(){return m_style & VISIBLE;}
  virtual void setVisible(bool value) {m_style = RENDER_STYLE((m_style | !VISIBLE) & (value?1:0));}
  virtual RENDER_STYLE style() {return m_style;}
  virtual pqOutputPort *outputPort() = 0;
  virtual pqPipelineSource *data() = 0;
  virtual int portNumber() = 0;
  
protected:
  RENDER_STYLE m_style;
  pqPipelineSource *m_source;
  int m_portNumber;
};


#endif // INTERFACES_H
