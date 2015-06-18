/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include "CountingFramePlugin_Export.h"

// VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkPolyData;

/// Base class for bounding region widgets
class CountingFramePlugin_EXPORT vtkCountingFrameWidget
: public vtkAbstractWidget
{
public:
  vtkTypeMacro(vtkCountingFrameWidget, vtkAbstractWidget);

  vtkGetVector3Macro(InclusionOffset, ESPINA::Nm);
  vtkGetVector3Macro(ExclusionOffset, ESPINA::Nm);

  /** \brief Sets the counting frame the widget will represent.
   * \param[in] region Counting frame region as a polydata.
   * \param[in] inclussionOffset
   * \param[in] exclussionOffset
   *
   */
  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                 ESPINA::Nm inclusionOffset[3],
                                 ESPINA::Nm exclusionOffset[3]) = 0;

  /** \brief Sets widget visibility.
   * \param[in] visible true to set visible and false otherwise.
   *
   */
  virtual void setVisible(bool visible) = 0;

  /** \brief Returns true if the widget is visible and false otherwise.
   *
   */
  virtual bool isVisible() const
  { return Visible; }

protected:
  ESPINA::Nm InclusionOffset[3];
  ESPINA::Nm ExclusionOffset[3];
  bool Visible;

protected:
  vtkCountingFrameWidget()
  : Visible{true}
  {
    memset(InclusionOffset, 0, 3*sizeof(ESPINA::Nm));
    memset(ExclusionOffset, 0, 3*sizeof(ESPINA::Nm));
  }
};

#endif // VTKBOUNDINGFRAMEWIDGET_H
