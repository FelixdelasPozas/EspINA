/*
 * vtkZoomSelectionWidget.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef VTKZOOMSELECTIONWIDGET_H_
#define VTKZOOMSELECTIONWIDGET_H_

// EspINA
#include "common/EspinaTypes.h"

// vtk
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkPoints;

class VTK_WIDGETS_EXPORT vtkZoomSelectionWidget
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
