/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef COLORENGINE_H
#define COLORENGINE_H

#include <QSharedPointer>

#include <QColor>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class Segmentation;

typedef vtkSmartPointer<vtkLookupTable> LUTPtr;

class ColorEngine
: public QObject
{
  Q_OBJECT

public:
  enum Components
  {
    None = 0x0, Color = 0x1, Transparency = 0x2
  };
  Q_DECLARE_FLAGS(Composition, Components)

public:
  virtual QColor color(Segmentation *seg) = 0;
  virtual LUTPtr lut(Segmentation *seg) = 0;

  virtual Composition supportedComposition() const = 0;

signals:
  void lutModified();
};

typedef QSharedPointer<ColorEngine> ColorEnginePtr;

Q_DECLARE_OPERATORS_FOR_FLAGS(ColorEngine::Composition)

#endif // COLORENGINE_H