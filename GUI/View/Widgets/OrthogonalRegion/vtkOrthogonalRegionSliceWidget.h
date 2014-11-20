/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_VTK_RECTANGULAR_SLICE_WIDGET_H
#define ESPINA_VTK_RECTANGULAR_SLICE_WIDGET_H

#include "EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>
#include "GUI/View/Widgets/OrthogonalRegion/OrthogonalRegion.h"

// VTK
#include "vtkAbstractWidget.h"

class vtkPolyData;
class vtkRectangularSliceRepresentation;

namespace ESPINA
{
  class View2D;

  class EspinaGUI_EXPORT vtkOrthogonalRegionSliceWidget
  : public vtkAbstractWidget
  {
  public:
    // Description:
    // Instantiate the object.
    static vtkOrthogonalRegionSliceWidget *New();

    // Description:
    // Standard class methods for type information and printing.
    vtkTypeMacro(vtkOrthogonalRegionSliceWidget, vtkAbstractWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    virtual void SetView(View2D *view);
    virtual void SetPlane(Plane plane);
    virtual void SetSlice(Nm pos);
    virtual void SetBounds(Bounds bounds);
    virtual Bounds GetBounds();

    // Description:
    // Create the default widget representation if one is not set. By default,
    // this is an instance of the vtkRectangularRectangularRepresentation class.
    void CreateDefaultRepresentation();

    // modify representation methods
    void setRepresentationColor(double *);
    void setRepresentationPattern(int);

  protected:
    vtkOrthogonalRegionSliceWidget();
    ~vtkOrthogonalRegionSliceWidget();

  private:
    void updateRepresentation();

  private:
    //BTX - manage the state of the widget
    int WidgetState;
    enum _WidgetState {Start=0,Active};
    //ETX

    // These methods handle events
    static void SelectAction(vtkAbstractWidget*);
    static void EndSelectAction(vtkAbstractWidget*);
    static void TranslateAction(vtkAbstractWidget*);
    static void MoveAction(vtkAbstractWidget*);

    // helper methods for cursoe management
    virtual void SetCursor(int state);

    Plane m_plane;
    Nm Slice;
    Bounds m_bounds;
    double m_color[3];
    int m_pattern;

  private:
    vtkOrthogonalRegionSliceWidget(const vtkOrthogonalRegionSliceWidget&);  //Not implemented
    void operator=(const vtkOrthogonalRegionSliceWidget&);  //Not implemented
  };

} // namespace ESPINA

#endif // ESPINA_VTK_RECTANGULAR_SLICE_WIDGET_H
