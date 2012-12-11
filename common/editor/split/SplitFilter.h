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


#ifndef SPLITFILTER_H
#define SPLITFILTER_H

#include <common/model/Filter.h>

#include <vtkSmartPointer.h>

class vtkImageStencilData;

/// Split Segmentation into two components according to
/// given stencil
class SplitFilter
: public Filter
{
public:
  static const QString TYPE;
  static const QString INPUTLINK;

public:
  explicit SplitFilter(NamedInputs inputs, Arguments args);
  virtual ~SplitFilter();

  // Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  // Implements Filter Interface
  virtual bool needUpdate() const;

  void setStencil(vtkSmartPointer<vtkImageStencilData> stencil)
  { m_stencil = stencil; }

protected:
  virtual void run();
  virtual bool prefetchFilter();

private:
  vtkSmartPointer<vtkImageStencilData> m_stencil;
};

#endif // SPLITFILTER_H
