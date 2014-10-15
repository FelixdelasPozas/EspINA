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

#ifndef VTKPLANARSPLITREPRESENTATION2D_H_
#define VTKPLANARSPLITREPRESENTATION2D_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// vtk
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>
#include <vtkWidgetRepresentation.h>

class vtkPoints;
class vtkLineSource;
class vtkActor;
class vtkPointHandleRepresentation2D;
class vtkHandleRepresentation;

namespace ESPINA
{
  class EspinaGUI_EXPORT vtkPlanarSplitRepresentation2D
  : public vtkWidgetRepresentation
  {
    vtkTypeMacro(vtkPlanarSplitRepresentation2D,vtkWidgetRepresentation);

  public:
    /** \brief Returns a new instance.
     *
     */
    static vtkPlanarSplitRepresentation2D *New();

    /** \brief Get the points that define the splitting segment.
     *
     */
    vtkSmartPointer<vtkPoints> getPoints();

    /** \brief Sets the points that define the splitting segment.
     * \param[in] points, vtkPoints smart pointer with two points.
     *
     */
    void setPoints(vtkSmartPointer<vtkPoints>);

    /** \brief Sets the first point.
     * \param[in] point, vector of three Nm values.
     *
     */
    void setPoint1(Nm *point);

    /** \brief Sets the second point.
     * \param[in] point, vector of three Nm values.
     *
     */
    void setPoint2(Nm *);

    /** \brief Returns the pointer to the first point of the segment.
     *
     */
    void getPoint1(Nm *);

    /** \brief Returns the pointer to the second point of the segment.
     *
     */
    void getPoint2(Nm *);

    /** \brief Sets the slice of the representation.
     * \param[in] slice, slice in Nm.
     *
     */
    void setSlice(double slice);

    /** \brief The tolerance representing the distance to the widget (in pixels) in
     * which the cursor is considered near enough to the end points of
     * the widget to be active.
     *
     */
    vtkSetClampMacro(m_tolerance,int,1,100);
    vtkGetMacro(m_tolerance,int);

    /** \brief Implements vtkWidgetRepresentation::BuildRepresentation().
     *
     */
    virtual void BuildRepresentation();

    /** \brief Overrides vtkWidgetRepresentation::ComputeInteractionState().
     *
     */
    virtual int ComputeInteractionState(int X, int Y, int modify=0) override;

    /** \brief Overrides vtkWidgetRepresentation::StartWidgetInteraction().
     *
     */
    virtual void StartWidgetInteraction(double e[2]) override;

    /** \brief Overrides vtkWidgetRepresentation::WidgetInteraction().
     *
     */
    virtual void WidgetInteraction(double e[2]) override;

    /** \brief Overrides vtkWidgetRepresentation::ReleaseGraphicsResources().
     *
     */
    virtual void ReleaseGraphicsResources(vtkWindow *w) override;

    /** \brief Overrides vtkWidgetRepresentation::RenderOverlay().
     *
     */
    virtual int RenderOverlay(vtkViewport *viewport) override;

    /** \brief Overrides vtkWidgetRepresentation::RenderOpaqueGeometry().
     *
     */
    virtual int RenderOpaqueGeometry(vtkViewport *viewport) override;

    enum { Outside = 0, NearP1, NearP2 };

    /** \brief This method is used to specify the type of handle representation to
     * use for the two internal vtkHandleWidgets within the widget.
     * To use this method, create a dummy vtkHandleWidget (or subclass),
     * and then invoke this method with this dummy. Then the
     * representation uses this dummy to clone two vtkHandleWidgets
     * of the same type. Make sure you set the handle representation before
     * the widget is enabled. (The method InstantiateHandleRepresentation()
     * is invoked by the widget.)
     *
     */
    void SetHandleRepresentation(vtkHandleRepresentation *handle);
    void InstantiateHandleRepresentation();
    void MoveHandle(int handleNum, int X, int Y);

    /** \brief Set/Get the two handle representations used for the widget. (Note:
     * properties can be set by grabbing these representations and setting the
     * properties appropriately.)
     *
     */
    vtkGetObjectMacro(Point1Representation,vtkHandleRepresentation);
    vtkGetObjectMacro(Point2Representation,vtkHandleRepresentation);

    /** \brief Sets the representation orientation.
     * \param[in] orientation, plane orientation,
     */
    virtual void setOrientation(Plane orientation);

    /** \brief Sets the distance value over the rest of the view's representations.
     * \param[in] shift view's widgets shift value.
     *
     */
    virtual void setShift(const Nm shift);

    /** \brief Sets the segmentation bounds to draw the actor.
     * \param[in] bounds, raw pointer to a vector of six double values.
     *
     */
    virtual void setSegmentationBounds(double *bounds);

    /** \brief Removes the bounds actor from the view.
     *
     */
    virtual void removeBoundsActor();

  protected:
    /** \brief vtkPlanarSplitRepresentation2D class constructor.
     *
     */
    vtkPlanarSplitRepresentation2D();

    /** \brief vtkPlanarSplitRepresentation2D class destructor.
     *
     */
    ~vtkPlanarSplitRepresentation2D();

    double m_point1[3];
    double m_point2[3];
    vtkSmartPointer<vtkLineSource> m_line;
    vtkSmartPointer<vtkActor> m_lineActor;
    vtkSmartPointer<vtkPoints> m_boundsPoints;
    vtkSmartPointer<vtkActor> m_boundsActor;

    vtkHandleRepresentation *HandleRepresentation;
    vtkHandleRepresentation *Point1Representation;
    vtkHandleRepresentation *Point2Representation;

    int    m_tolerance; // Selection tolerance for the handles
    Plane  m_plane;
    Nm     m_epsilon;
    Nm     m_slice;
  private:
    /** \brief vtkPlanarSplitRepresentation2D copy constructor not implemented.
     *
     */
    vtkPlanarSplitRepresentation2D(const vtkPlanarSplitRepresentation2D&);

    /** \brief Assignment not implemented.
     *
     */
    void operator=(const vtkPlanarSplitRepresentation2D&);  //Not implemented
  };

} // namespace ESPINA

#endif /* VTKPLANARSPLITREPRESENTATION2D_H_ */
