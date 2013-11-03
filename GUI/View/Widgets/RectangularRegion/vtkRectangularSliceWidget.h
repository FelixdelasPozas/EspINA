/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "vtkAbstractWidget.h"

#include <Core/EspinaTypes.h>
#include "GUI/View/Widgets/RectangularRegion/RectangularRegion.h"

class vtkPolyData;
class vtkRectangularSliceRepresentation;

namespace EspINA
{

  class EspinaGUI_EXPORT vtkRectangularSliceWidget
  : public vtkAbstractWidget
  {
  public:
    // Description:
    // Instantiate the object.
    static vtkRectangularSliceWidget *New();

    // Description:
    // Standard class methods for type information and printing.
    vtkTypeMacro(vtkRectangularSliceWidget, vtkAbstractWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    virtual void SetPlane(Plane plane);
    virtual void SetSlice(Nm pos);
    virtual void SetBounds(double bounds[6]);
    virtual void GetBounds(double bounds[6]);

    // Description:
    // Create the default widget representation if one is not set. By default,
    // this is an instance of the vtkRectangularRectangularRepresentation class.
    void CreateDefaultRepresentation();

    // modify representation methods
    void setRepresentationColor(double *);
    void setRepresentationPattern(int);

  protected:
    vtkRectangularSliceWidget();
    ~vtkRectangularSliceWidget();

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
    double Bounds[6];
    double m_color[3];
    int m_pattern;

  private:
    vtkRectangularSliceWidget(const vtkRectangularSliceWidget&);  //Not implemented
    void operator=(const vtkRectangularSliceWidget&);  //Not implemented
  };

} // namespace EspINA

#endif // ESPINA_VTK_RECTANGULAR_SLICE_WIDGET_H
