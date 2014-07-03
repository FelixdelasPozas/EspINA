/*
 * vtkPlanarSplitWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef VTKPLANARSPLITWIDGET_H_
#define VTKPLANARSPLITWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkHandleWidget;
class vtkWidgetRepresentation;
class vtkPoints;
class vtkLineSource;

namespace EspINA
{
  class vtkPlanarSplitWidgetCallback;
  class vtkPlanarSplitRepresentation2D;

  class EspinaGUI_EXPORT vtkPlanarSplitWidget
  : public vtkAbstractWidget
  {
  public:

    static vtkPlanarSplitWidget *New();

    vtkTypeMacro(vtkPlanarSplitWidget,vtkAbstractWidget);

    // The method for activating and deactivating this widget. This method
    // must be overridden because it is a composite widget and does more than
    // its superclasses' vtkAbstractWidget::SetEnabled() method.
    virtual void SetEnabled(int);

    // Description:
    // Specify an instance of vtkWidgetRepresentation used to represent this
    // widget in the scene. Note that the representation is a subclass of vtkProp
    // so it can be added to the renderer independent of the widget.
    void SetRepresentation(vtkPlanarSplitRepresentation2D *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

    // Description:
    // Methods to change the whether the widget responds to interaction.
    // Overridden to pass the state to component widgets.
    virtual void SetProcessEvents(int);

    // Description:
    // Create the default widget representation if one is not set.
    void CreateDefaultRepresentation();

    // Description:
    // Enum defining the state of the widget. By default the widget is in Start mode,
    // and expects to be interactively placed. While placing the points the widget
    // transitions to Define state. Once placed, the widget enters the Manipulate state.
    //BTX
    enum {Start=0,Define,Manipulate};
    //ETX

    // Description:
    // Set the state of the widget. If the state is set to "Manipulate" then it
    // is assumed that the widget and its representation will be initialized
    // programmatically and is not interactively placed. Initially the widget
    // state is set to "Start" which means nothing will appear and the user
    // must interactively place the widget with repeated mouse selections. Set
    // the state to "Start" if you want interactive placement. Generally state
    // changes must be followed by a Render() for things to visually take
    // effect.
    virtual void SetWidgetStateToStart();
    virtual void SetWidgetStateToManipulate();

    // Description:
    // Return the current widget state.
    virtual int GetWidgetState()
    {return this->WidgetState;}

    // get/set widget data
    virtual void setPoints(vtkSmartPointer<vtkPoints>);
    vtkSmartPointer<vtkPoints> getPoints();

    // set widget orientation
    virtual void setOrientation(Plane orientation);
    virtual Plane getOrientation() const
    { return m_plane; }

    virtual void PrintSelf(ostream &os, vtkIndent indent);

    virtual void disableWidget();
    virtual void setSegmentationBounds(double *bounds);

    virtual void setSlice(double slice);

  protected:
    vtkPlanarSplitWidget();
    virtual ~vtkPlanarSplitWidget();

    // The state of the widget
    int WidgetState;
    int CurrentHandle;
    Plane  m_plane;
    double m_segmentationBounds[6];
    double m_slice;

    // Callback interface to capture events when
    // placing the widget.
    static void AddPointAction(vtkAbstractWidget*);
    static void MoveAction(vtkAbstractWidget*);
    static void EndSelectAction(vtkAbstractWidget*);

    // The positioning handle widgets
    vtkHandleWidget *m_point1Widget;
    vtkHandleWidget *m_point2Widget;
    vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback1;
    vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback2;

    // Method invoked when the handles at the
    // end points of the widget are manipulated
    void StartHandleInteraction(int handleNum);
    void HandleInteraction(int handleNum);
    void StopHandleInteraction(int handleNum);

    //BTX
    friend class vtkPlanarSplitWidgetCallback;
    //ETX

    bool m_permanentlyDisabled;
  };

} // namespace EspINA

#endif /* VTKPLANARSPLITWIDGET_H_ */
