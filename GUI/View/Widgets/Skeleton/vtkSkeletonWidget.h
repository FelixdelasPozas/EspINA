/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VTK_SKELETON_WIDGET_H_
#define ESPINA_VTK_SKELETON_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt
#include <QCursor>
#include <QColor>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Skeleton
        {
          class vtkSkeletonWidgetRepresentation;

          /** \class vtkSkeletonWidget
           * \brief VTK widget for skeleton interactions.
           *
           */
          class EspinaGUI_EXPORT vtkSkeletonWidget
          : public vtkAbstractWidget
          {
            public:
              /** \brief Creates a new instance.
               *
               */
              static vtkSkeletonWidget *New();

              vtkTypeMacro(vtkSkeletonWidget,vtkAbstractWidget);

              virtual void SetEnabled(int) override;

              void CreateDefaultRepresentation() override;

              virtual void PrintSelf(ostream &os, vtkIndent indent) override;

              /** \brief Convenient method to determine the state of the method.
               *
               */
              int GetWidgetState()
              { return m_widgetState; }

              /** \brief Sets the orientation of the widget.
               * \param[in] plane orientation plane.
               *
               */
              virtual void SetOrientation(Plane plane);

              /** \brief Returns the widget planar orientation.
               *
               */
              const Plane &orientation() const
              { return m_orientation; }

              /** \brief Sets the slice for the representation of the widget if it's from the same orientation.
               * \param[in] plane orientation plane.
               * \param[in] value slice value.
               *
               */
              void changeSlice(Plane plane, Nm value);

              /** \brief Returns the skeleton.
               *
               */
              vtkSmartPointer<vtkPolyData> getSkeleton();

              /** \brief Sets the spacing of the view in the Z coordinate to draw the representation correctly.
               * \param[in] shift distance in Nm.
               *
               */
              void SetShift(const Nm shift);

              /** \brief Returns the spacing of the view in the Z coordinate to draw the representation correctly.
               *
               */
              const Nm shift() const
              { return m_shift; }

              /** \brief Sets the spacing of the representation to make the position of all nodes of the
               * representation centered on voxel center.
               * \param[in] spacing Spacing vector.
               *
               */
              void SetSpacing(const NmVector3 &spacing);

              /** \brief Returns the spacing of the widget.
               *
               */
              const NmVector3 &spacing() const
              { return m_spacing; }

              /** \brief Updates the representation.
               *
               */
              void UpdateRepresentation();

              /** \brief Operation modes. */
              enum { Define = 0, Delete = 1, Manipulate = 2, Mark = 3 };

              /** \brief Adds a point to the skeleton in the event coordinates position.
               *
               */
              void addPoint();

              /** \brief Moves the currently selected node to the given event coordinates position.
               *
               */
              void movePoint();

              /** \brief Stops the current operation.
               *
               */
              void stop();

              /** \brief Removes the current node or does nothing if there is no node selected. Returns true if a node was delete and false otherwise.
               *
               */
              bool deletePoint();

              /** \brief Selects the closest node to the given mouse coordinates if is between tolerance values.
               * Returns true on success and false otherwise.
               *
               */
              bool selectNode();

              /** \brief Returns the number of nodes in the current representation.
               *
               */
              const unsigned int numberOfPoints() const;

              /** \brief Sets the value of the "ignore cursor" flag.
               * \param[in] value true to ignore the cursor in distances computations and false otherwise.
               *
               */
              void setIgnoreCursor(bool value)
              { m_ignoreCursor = value; }

              /** \brief Returns the value of the "ignore cursor flag". If true means that a create operation is not finished.
               *
               */
              bool ignoreCursor() const
              { return m_ignoreCursor; }

              /** \brief Removes all the nodes.
               *
               */
              void ClearRepresentation();

              /** \brief Sets the current operation mode of the widget.
               * \param[in] mode integer of operation mode.
               *
               */
              void setCurrentOperationMode(const int mode);

              /** \brief Returns the integer of the current operation mode of the widget.
               *
               */
              const int currentOperationMode() const;

              /** \brief Computes the interaction state and updates the mouse cursor.
               *
               */
              void updateCursor();

              /** \brief Callback method to rebuild the representation when a change occurs in the representation.
               *
               */
              void BuildRepresentation();

              /** \brief Sets the stroke of the next points.
               * \param[in] stroke skeleton stroke struct.
               *
               */
              void setStroke(const Core::SkeletonStroke &stroke);

              /** \brief Removes the stroke from the skeleton (also removes all edges of the stroke).
               * \param[in] stroke skeleton stroke struct.
               *
               */
              void removeStroke(const Core::SkeletonStroke &stroke);

              /** \brief Renames the stroke with the old name with the new one.
               * \param[in] oldName Stroke current name.
               * \param[in] newName Stroke new name.
               *
               */
              void renameStroke(const QString &oldName, const QString &newName);

              /** \brief Returns the current stroke definition used.
               *
               */
              const Core::SkeletonStroke stroke() const;

              /** \brief Using the last three points connects the first and last with the current stroke and creates
               *  a connection using the given stroke, the second point and the closest point to the first-last segment.
               *  Returns true on success and false if a connection cannot be created (less than three nodes in the skeleton).
               *  \param[in] stroke Stroke of the connection.
               *
               */
              void createConnection(const Core::SkeletonStroke &stroke);

              /** \brief Changes the stroke to the given one after adding a new point.
               *  \param[in] stroke New stroke.
               *
               */
              void changeStroke(const Core::SkeletonStroke &stroke);

              /** \brief Returns true if the given point will be considered collision with an existing stroke.
               * \param[in] point 3D point coordinates.
               *
               */
              bool isStartNode(const NmVector3 &point) const;

              /** \brief Returns true if the branch of the current node could be marked as truncated, false otherwise.
               *
               */
              bool markAsTruncated();

              /** \brief Sets the width of the skeleton representation.
               * \param[in] width Integer value.
               *
               */
              void setRepresentationWidth(int width);

              /** \brief Enables/disables the annotations of the skeleton.
               * \param[in] value true to enable and false to disable.
               *
               */
              void setRepresentationShowText(bool value);

              /** \brief Sets the size of the skeleton annotations.
               * \param[in] size Integer value.
               *
               */
              void setRepresentationTextSize(int size);

              /** \brief Sets the color of the text labels background.
               * \param[in] color QColor object.
               *
               */
              void setRepresentationTextColor(const QColor &color);

              /** \brief Returns the paths that contains the currently selected node.
               *
               */
              const Core::PathList selectedPaths() const;

              /** \brief Enables/disables modifications in hue for the strokes that coincide in color to facilitate skeleton
               * visualization.
               * \param[in] value True to modify hue value of strokes of the same color and false to draw the strokes with its assigned hue.
               *
               */
              void setStrokeHueModification(const bool value);

              /** \brief Returns true if the hue value of strokes coincident in color is being modified to failitate visualization. Returns
               * false if not.
               *
               */
              const bool strokeHueModification() const;

              /** \brief Sets the default color of the representation.
               * \param[in] value Hue value in (0-359).
               *
               */
              void setDefaultHue(const int value);

            protected:
              int       m_widgetState;   /** widget operation state.                */
              Plane     m_orientation;   /** orthogonal plane of the widget.        */
              Nm        m_slice;         /** current slice position in Nm.          */
              Nm        m_shift;         /** actor's shift over the slice position. */
              NmVector3 m_spacing;       /** representation's spacing.              */

              /** \brief Overrides vtkAbstractWidget::cursor().
               *
               */
              virtual void SetCursor(int State) override;

            protected:
              /** \brief vtkSkeletonWidget class constructor.
               *
               */
              vtkSkeletonWidget();

              /** \brief vtkSkeletonWidget class virtual destructor.
               *
               */
              virtual ~vtkSkeletonWidget();

            private:
              /** \brief Helper method to create the cursors for this widget.
               *
               */
              void createCursors();

              /** \brief vtkSkeletonWidget class copy constructor (not implemented).
               *
               */
              vtkSkeletonWidget(const vtkSkeletonWidget&);

              /** \brief Assignment operator not implemented.
               *
               */
              void operator=(const vtkSkeletonWidget&);

              /** \brief Helper method to obtain the representation pointer.
               *
               */
              vtkSkeletonWidgetRepresentation *representation();

              QCursor m_crossMinusCursor; /** cross minus cursor shown when deleting nodes.                                         */
              QCursor m_crossPlusCursor;  /** cross plus cursor shown when adding nodes between existing nodes.                     */
              QCursor m_crossCheckCursor; /** cross check cursor shown when connecting two strokes.                                 */
              QCursor m_truncateCursor;   /** cursor for mark stroke operation.                                                     */
              bool    m_ignoreCursor;     /** true to ignore the current cursor node in distances computations and false otherwise. */
          };

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_H_
