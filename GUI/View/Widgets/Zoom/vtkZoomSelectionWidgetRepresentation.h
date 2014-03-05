/*
 * vtkZoomSelectionWidgetRepresentation.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef VTKZOOMSELECTIONWIDGETREPRESENTATION_H_
#define VTKZOOMSELECTIONWIDGETREPRESENTATION_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "vtkZoomSelectionWidget.h"

// VTK
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkActor;

class EspinaGUI_EXPORT vtkZoomSelectionWidgetRepresentation
: public vtkWidgetRepresentation
{
  vtkTypeMacro(vtkZoomSelectionWidgetRepresentation,vtkWidgetRepresentation);

  public:
    static vtkZoomSelectionWidgetRepresentation *New();

    virtual void SetWidgetType(vtkZoomSelectionWidget::WidgetType type);


    // Description:
    // These are methods that satisfy vtkWidgetRepresentation's API.
    virtual void BuildRepresentation();
    virtual int ComputeInteractionState(int X, int Y, int modify = 0);
    virtual void StartWidgetInteraction(double e[2]);
    virtual void WidgetInteraction(double e[2]);
    virtual void EndWidgetInteraction(double e[2]);

    // Description:
    // Methods required by vtkProp superclass.
    virtual void ReleaseGraphicsResources(vtkWindow *w);
    virtual int RenderOverlay(vtkViewport *viewport);
    virtual int RenderOpaqueGeometry(vtkViewport *viewport);

  protected:
    vtkZoomSelectionWidgetRepresentation();
    virtual ~vtkZoomSelectionWidgetRepresentation();

    // attributes
    vtkZoomSelectionWidget::WidgetType m_type;
    vtkSmartPointer<vtkPoints> m_displayPoints;
    vtkSmartPointer<vtkPoints> m_worldPoints;
    vtkSmartPointer<vtkActor> m_lineActor;

  private:
    // helper methods
    void DisplayPointsToWorldPoints();

    vtkZoomSelectionWidgetRepresentation(const vtkZoomSelectionWidgetRepresentation&);  //Not implemented
    void operator=(const vtkZoomSelectionWidgetRepresentation&);  //Not implemented
};

#endif /* VTKZOOMSELECTIONWIDGETREPRESENTATION_H_ */
