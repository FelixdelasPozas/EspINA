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


#ifndef VTKBOUNDINGREGIONWIDGET_H
#define VTKBOUNDINGREGIONWIDGET_H

#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <common/EspinaTypes.h>

/// Base class for bounding region widgets
class VTK_WIDGETS_EXPORT vtkBoundingRegionWidget
: public vtkAbstractWidget
{
public:
  vtkTypeMacro(vtkBoundingRegionWidget, vtkAbstractWidget);

  vtkSetVector3Macro(InclusionOffset, Nm);
  vtkGetVector3Macro(InclusionOffset, Nm);
  vtkSetVector3Macro(ExclusionOffset, Nm);
  vtkGetVector3Macro(ExclusionOffset, Nm);

  virtual void SetBoundingRegion(vtkSmartPointer<vtkPolyData> region) = 0;

protected:
  Nm InclusionOffset[3];
  Nm ExclusionOffset[3];
};

#endif // VTKBOUNDINGREGIONWIDGET_H
