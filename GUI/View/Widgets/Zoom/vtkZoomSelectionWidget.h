/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef VTKZOOMSELECTIONWIDGET_H_
#define VTKZOOMSELECTIONWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkPoints;

class EspinaGUI_EXPORT vtkZoomSelectionWidget
: public vtkAbstractWidget
{
  public:
    static vtkZoomSelectionWidget *New();

    vtkTypeMacro(vtkZoomSelectionWidget,vtkAbstractWidget);

    // The method for activating and deactivating this widget. This method
    // must be overridden because it is a composite widget and does more than
    // its superclasses' vtkAbstractWidget::SetEnabled() method.
    virtual void SetEnabled(int);

    // Description:
    // Specify an instance of vtkWidgetRepresentation used to represent this
    // widget in the scene. Note that the representation is a subclass of vtkProp
    // so it can be added to the renderer independent of the widget.
    void SetRepresentation(vtkWidgetRepresentation *r)
          { this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

    // Description:
    // Create the default widget representation if one is not set.
    void CreateDefaultRepresentation();

    // Description:
    // Enum defining the state of the widget. By default the widget is in Start mode,
    // and expects to be interactively placed. While placing the points the widget
    // transitions to Define state. Once placed, the widget enters the Manipulate state.
    //BTX
    enum {Start=0,Define, End};
    //ETX

    // Description:
    // Return the current widget state.
    virtual int GetWidgetState() {return this->WidgetState;}

    enum WidgetType { AXIAL_WIDGET = 2, CORONAL_WIDGET = 1, SAGITTAL_WIDGET = 0, VOLUME_WIDGET = 3, NONE = 4 };
    virtual void SetWidgetType(WidgetType);

  protected:
    vtkZoomSelectionWidget();
    virtual ~vtkZoomSelectionWidget();

    // Callback interface to capture events
    static void SelectAction(vtkAbstractWidget*);
    static void MoveAction(vtkAbstractWidget*);
    static void EndSelectAction(vtkAbstractWidget*);

    int WidgetState;
    WidgetType m_type;
};

#endif /* VTKZOOMSELECTIONWIDGET_H_ */
