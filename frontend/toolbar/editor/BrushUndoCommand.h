/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef BRUSHUNDOCOMMAND_H
#define BRUSHUNDOCOMMAND_H

#include "Brush.h"
#include <QUndoCommand>

class Filter;
class vtkImplicitFunction;

class Brush::DrawCommand
: public QUndoCommand
{
public:
  explicit DrawCommand(Filter *source,
                       Filter::OutputId output,
                       BrushShapeList brushes,
                       EspinaVolume::PixelType value);
  virtual void redo();
  virtual void undo();

private:
  Filter        *m_source;
  Filter::OutputId   m_output;
  BrushShapeList m_brushes;

  double m_strokeBounds[6];

  EspinaVolume::PixelType m_value;
  EspinaVolume::Pointer m_prevVolume;
  EspinaVolume::Pointer m_newVolume;
};

class Brush::SnapshotCommand
: public QUndoCommand
{
public:
  explicit SnapshotCommand(Filter *source,
                       Filter::OutputId output);

  virtual void redo();
  virtual void undo();

private:
  Filter      *m_source;
  Filter::OutputId m_output;

  EspinaVolume::Pointer m_prevVolume;
  EspinaVolume::Pointer m_newVolume;
};

#endif // BRUSHUNDOCOMMAND_H