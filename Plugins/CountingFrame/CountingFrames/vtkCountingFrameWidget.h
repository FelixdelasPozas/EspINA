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


#ifndef VTKBOUNDINGFRAMEWIDGET_H
#define VTKBOUNDINGFRAMEWIDGET_H

#include "CountingFramePlugin_Export.h"

#include <Core/EspinaTypes.h>

#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

/// Base class for bounding region widgets
class CountingFramePlugin_EXPORT vtkCountingFrameWidget
: public vtkAbstractWidget
{
public:
  vtkTypeMacro(vtkCountingFrameWidget, vtkAbstractWidget);

  vtkGetVector3Macro(InclusionOffset, EspINA::Nm);
  vtkGetVector3Macro(ExclusionOffset, EspINA::Nm);

  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                 EspINA::Nm inclusionOffset[3],
                                 EspINA::Nm exclusionOffset[3]) = 0;

protected:
  EspINA::Nm InclusionOffset[3];
  EspINA::Nm ExclusionOffset[3];

protected:
  vtkCountingFrameWidget()
  {
    memset(InclusionOffset, 0, 3*sizeof(EspINA::Nm));
    memset(ExclusionOffset, 0, 3*sizeof(EspINA::Nm));
  }
};

#endif // VTKBOUNDINGFRAMEWIDGET_H
