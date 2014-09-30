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

#ifndef VTKPLANARSPLITWIDGET_H_
#define VTKPLANARSPLITWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkHandleWidget;
class vtkWidgetRepresentation;
class vtkPoints;
class vtkLineSource;

namespace ESPINA
{
  class vtkPlanarSplitWidgetCallback;
  class vtkPlanarSplitRepresentation2D;

  class EspinaGUI_EXPORT vtkPlanarSplitWidget
  : public vtkAbstractWidget
  {
  public:
  	/** \brief Creates a new instance.
  	 *
  	 */
    static vtkPlanarSplitWidget *New();

    vtkTypeMacro(vtkPlanarSplitWidget,vtkAbstractWidget);

    /** \brief Overrides vtkAbstractWidget::SetEnabled().
     *
     */
    virtual void SetEnabled(int) override;

  	/** \brief Specify an instance of vtkWidgetRepresentation used to represent this
  	 * widget in the scene. Note that the representation is a subclass of vtkProp
  	 * so it can be added to the renderer independent of the widget.
  	 *
  	 */
    void SetRepresentation(vtkPlanarSplitRepresentation2D *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

    /** \brief Overrides vtkAbstractWidget::SetProcessEvents().
     *
     */
    virtual void SetProcessEvents(int) override;

  	/** \brief Implements vtkAbstractWidget::CreateDefaultRepresentation().
  	 *
  	 */
    void CreateDefaultRepresentation();

  	/** \brief Enum defining the state of the widget. By default the widget is in Start mode,
  	 * and expects to be interactively placed. While placing the points the widget
  	 * transitions to Define state. Once placed, the widget enters the Manipulate state.
  	 *
  	 */
    //BTX
    enum {Start=0,Define,Manipulate};
    //ETX

  	/** \brief Set the state of the widget. If the state is set to "Manipulate" then it
  	 * is assumed that the widget and its representation will be initialized
  	 * programmatically and is not interactively placed. Initially the widget
  	 * state is set to "Start" which means nothing will appear and the user
  	 * must interactively place the widget with repeated mouse selections. Set
  	 * the state to "Start" if you want interactive placement. Generally state
  	 * changes must be followed by a Render() for things to visually take
  	 * effect.
  	 *
  	 */
    virtual void SetWidgetStateToStart();
    virtual void SetWidgetStateToManipulate();

  	/** \brief Return the current widget state.
  	 *
  	 */
    virtual int GetWidgetState()
    {return this->WidgetState;}

  	/** \brief Set the widget segment points.
  	 * \param[in] points.
  	 *
  	 */
    //
    virtual void setPoints(vtkSmartPointer<vtkPoints> points);

    /** \brief Returns the points of the segment.
     *
     */
    vtkSmartPointer<vtkPoints> getPoints();

  	/** \brief Sets widget orientation.
  	 * \param[in] orientantion, orientation plane.
  	 *
  	 */
    virtual void setOrientation(Plane orientation);

    /** \brief Returns the widget orientation.
     *
     */
    virtual Plane getOrientation() const
    { return m_plane; }

  	/** \brief Overrides vtkAbstractWidget::PrintSelf().
  	 *
  	 */
    virtual void PrintSelf(ostream &os, vtkIndent indent) override;

  	/** \brief Disables the widget.
  	 *
  	 */
    virtual void disableWidget();

  	/** \brief Sets the segmentations bounds to draw the widget.
  	 * \param[in] bounds, pointer to a vector of six double values.
  	 *
  	 */
    virtual void setSegmentationBounds(double *bounds);

  	/** \brief Sets the slice of the widget.
  	 * \param[in] slice, slice of the view of the widget.
  	 *
  	 */
    virtual void setSlice(double slice);

  protected:
  	/** \brief vtkPlanarSplitWidget class constructor.
  	 *
  	 */
    vtkPlanarSplitWidget();

  	/** \brief vtkPlanarSplitWidget class destructor.
  	 *
  	 */
    virtual ~vtkPlanarSplitWidget();

    // The state of the widget
    int WidgetState;
    int CurrentHandle;
    Plane  m_plane;
    double m_segmentationBounds[6];
    double m_slice;

    /** \brief Callback interface to capture events when
     * placing the widget.
     *
     */
    static void AddPointAction(vtkAbstractWidget*);
    static void MoveAction(vtkAbstractWidget*);
    static void EndSelectAction(vtkAbstractWidget*);

    // The positioning handle widgets
    vtkHandleWidget *m_point1Widget;
    vtkHandleWidget *m_point2Widget;
    vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback1;
    vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback2;

  	/** \brief Method invoked when the handles at the
  	 * end points of the widget are manipulated
  	 *
  	 */
    void StartHandleInteraction(int handleNum);
    void HandleInteraction(int handleNum);
    void StopHandleInteraction(int handleNum);

    friend class vtkPlanarSplitWidgetCallback;

    bool m_permanentlyDisabled;
  };

} // namespace ESPINA

#endif /* VTKPLANARSPLITWIDGET_H_ */
