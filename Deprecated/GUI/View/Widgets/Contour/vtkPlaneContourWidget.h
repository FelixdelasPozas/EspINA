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

#ifndef _VTKPLANECONTOURWIDGET_H_
#define _VTKPLANECONTOURWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Selectors/BrushSelector.h>

// VTK
#include <vtkAbstractWidget.h>

// Qt
#include <QCursor>
#include <QColor>

class vtkPolyData;

namespace ESPINA
{
  class vtkSliceContourRepresentation;
  class ContourWidget;

  class EspinaGUI_EXPORT vtkPlaneContourWidget
  : public vtkAbstractWidget
  {
  public:
    /* \brief Instantiate this class.
     *
     */
    static vtkPlaneContourWidget * New();

    /* \brief Standard methods for a VTK class.
     *
     */
    vtkTypeMacro(vtkPlaneContourWidget,vtkAbstractWidget);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /* \brief The method for activating and de-activating this widget. This method
     *  must be overridden because it is a composite widget and does more than
     *  its superclasses' vtkAbstractWidget::SetEnabled() method.
     */
    virtual void SetEnabled(int enable) override;

    /* \brief Specify an instance of vtkWidgetRepresentation used to represent this
     * widget in the scene. The representation is a subclass of vtkProp so it can be
     * added to the renderer independent of the widget.
     *
     */
    void SetRepresentation(vtkSliceContourRepresentation *r)
    { this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r)); }

    /* \brief Return the representation as a ContourRepresentation.
     *
     */
    vtkSliceContourRepresentation *GetContourRepresentation()
    { return reinterpret_cast<vtkSliceContourRepresentation*>(this->WidgetRep); }

    /* \brief Create the default widget representation if one is not set.
     *
     */
    void CreateDefaultRepresentation();

    /* \brief Convenience method to close the contour loop.
     *
     */
    void CloseLoop();

    /* \brief Convenient method to change what state the widget is in.
     *
     */
    vtkSetMacro(WidgetState,int);

    /* \brief Convenient method to determine the state of the method.
     *
     */
    vtkGetMacro(WidgetState,int);

    /* \brief Set / Get the AllowNodePicking value. This ivar indicates whether the nodes
     * and points between nodes can be picked/un-picked by Ctrl+Click on the node.
     *
     */
    void SetAllowNodePicking(int);vtkGetMacro( AllowNodePicking, int );
    vtkBooleanMacro( AllowNodePicking, int );

    /* \brief Follow the cursor ? If this is ON, during definition, the last node of the
     * contour will automatically follow the cursor, without waiting for the the
     * point to be dropped. This may be useful for some interpolators, such as the
     * live-wire interpolator to see the shape of the contour that will be placed
     * as you move the mouse cursor.
     */
    vtkSetMacro( FollowCursor, int );
    vtkGetMacro( FollowCursor, int );
    vtkBooleanMacro( FollowCursor, int );

    /* \brief Define a contour by continuously drawing with the mouse cursor.
     * Press and hold the left mouse button down to continuously draw.
     * Releasing the left mouse button switches into a snap drawing mode.
     * Terminate the contour by pressing the right mouse button.  If you
     * do not want to see the nodes as they are added to the contour, set the
     * opacity to 0 of the representation's property.  If you do not want to
     * see the last active node as it is being added, set the opacity to 0
     * of the representation's active property.
     *
     */
    vtkSetMacro( ContinuousDraw, int );
    vtkGetMacro( ContinuousDraw, int );
    vtkBooleanMacro( ContinuousDraw, int );

    vtkGetMacro(ContinuousDrawTolerance, double);
    vtkSetMacro(ContinuousDrawTolerance, double);

    /* \brief Initialize the contour widget from a user supplied set of points. The
     * state of the widget decides if you are still defining the widget, or
     * if you've finished defining (added the last point) are manipulating
     * it. If the polydata supplied is closed, the state will be set to manipulate.
     * State: Define = 0, Manipulate = 1.
     *
     */
    virtual void Initialize(vtkPolyData * poly, int state = 1);
    virtual void Initialize()
    { this->Initialize(nullptr); }

    /* \brief Sets the orientation of the contour.
     * \param[in] plane, orientation plane.
     *
     */
    virtual void SetOrientation(Plane plane);

    /* \brief Returns the orientation of the contour.
     *
     */
    virtual Plane GetOrientation();

    /* \brief Sets the contour polygon color.
     *
     */
    virtual void setPolygonColor(QColor);

    /* \brief Returns the contour polygon color.
     *
     */
    virtual QColor getPolygonColor();

    /* \brief Sets the parent ContourWidget for this vtk widget.
     * \param[in] parent, ContourWidget raw pointer.
     *
     * Parent needed to signal start/end of a contour.
     *
     */
    void setContourWidget(ContourWidget *parent) { m_parent = parent; }

    /* \brief Sets the contour mode.
     * \param[in] mode, brush erase/draw mode.
     *
     */
    void setContourMode(BrushSelector::BrushMode mode);

    /* \brief Returns the contour mode.
     *
     */
    BrushSelector::BrushMode getContourMode();

    /* \brief Used by the slice widget to set the mode of a previosly stored contour.
     *
     */
    void setActualContourMode(Brush::BrushMode mode);

  protected:
    /* \brief vtkPlaneContourWidget class constructor.
     *
     */
    vtkPlaneContourWidget();

    /* \brief vtkPlaneContourWidget class virtual destructor.
     *
     */
    virtual ~vtkPlaneContourWidget();

    enum
    {
      Start, Define, Manipulate
    };

    //ETX
    int WidgetState;
    int CurrentHandle;
    int AllowNodePicking;
    int FollowCursor;
    int ContinuousDraw;
    int ContinuousActive;
    ESPINA::PlaneType Orientation;
    double ContinuousDrawTolerance;

    /* \brief Callback interface to capture events when placing the widget.
     *
     */
    static void SelectAction(vtkAbstractWidget*);
    static void AddFinalPointAction(vtkAbstractWidget*);
    static void MoveAction(vtkAbstractWidget*);
    static void EndSelectAction(vtkAbstractWidget*);
    static void DeleteAction(vtkAbstractWidget*);
    static void TranslateContourAction(vtkAbstractWidget*);
    static void ScaleContourAction(vtkAbstractWidget*);
    static void ResetAction(vtkAbstractWidget*);
    static void KeyPressAction(vtkAbstractWidget *);

    /* \brief Adds the current mouse position as a node.
     *
     */
    void AddNode();

    /* \brief Overrides vtkAbstractWidget::cursor().
     *
     */
    virtual void SetCursor(int State) override;

    /* \brief Helper method to avoid creating too many points in continuos drawing.
     * \param[in] x, x coordinante.
     * \param[in] y, y coordinate.
     *
     */
    virtual bool IsPointTooClose(int x,int y);

    /* \brief Find closest node to the cursor node.
     *
     */
    int FindClosestNode();

  private:
    /* \brief vtkPlaneContourWidget copy constructor not implemented.
     *
     */
    vtkPlaneContourWidget(const vtkPlaneContourWidget&);

    /* \brief Assignment operator not implemented.
     *
     */
    void operator=(const vtkPlaneContourWidget&);

    QCursor crossMinusCursor, crossPlusCursor, crossCheckCursor;
    bool mouseButtonDown; /// to create almost equally spaced points when using continuous drawing
    QColor m_polygonColor;
    ContourWidget *m_parent;
    Brush::BrushMode m_contourMode;
    Brush::BrushMode m_actualBrushMode;
  };

} // namespace ESPINA

#endif // _VTKPLANECONTOURWIDGET_H_
