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

#ifndef VTKBOUNDINGFRAME3DWIDGET_H
#define VTKBOUNDINGFRAME3DWIDGET_H

#include "CountingFramePlugin_Export.h"

// ESPINA
#include "vtkCountingFrameWidget.h"

class CountingFramePlugin_EXPORT vtkCountingFrame3DWidget
: public vtkCountingFrameWidget
{
  public:
    // Description:
    // Instantiate the object.
    static vtkCountingFrame3DWidget *New();

    // Description:
    // Standard class methods for type information and printing.
    vtkTypeMacro(vtkCountingFrame3DWidget, vtkCountingFrameWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    virtual void SetCountingFrame(vtkSmartPointer< vtkPolyData > region, ESPINA::Nm inclusionOffset[3], ESPINA::Nm exclusionOffset[3], ESPINA::NmVector3 resolution);

    // Description:
    // Create the default widget representation if one is not set. By default,
    // this is an instance of the vtkCountingFrame3DRepresentation class.
    void CreateDefaultRepresentation();

    virtual void setVisible(bool visible);

    /** \brief Sets the opacity of the representation.
     * \param[in] opacity opacity value in range [0,1].
     *
     */
    void SetOpacity(const float opacity);

    /** \brief Returns the opacity value of the representation in range [0,1].
     *
     */
    const float GetOpacity() const;

  protected:
    vtkCountingFrame3DWidget();
    ~vtkCountingFrame3DWidget();

  //BTX - manage the state of the widget
    int WidgetState;
    enum _WidgetState {Start=0,Active};
  //ETX

  private:
    vtkCountingFrame3DWidget(const vtkCountingFrame3DWidget&);  //Not implemented
    void operator=(const vtkCountingFrame3DWidget&);  //Not implemented
};

#endif // VTKBOUNDINGFRAME3DWIDGET_H
