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
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>
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

              /** \brief Convenient method to change what state the widget is in.
               * \param[in] state enumerated state of the widget.
               *
               */
              void SetWidgetState(int state);

              /** \brief Convenient method to determine the state of the method.
               *
               */
              int GetWidgetState()
              { return m_widgetState; }

              /** \brief Initialize the skeleton widget from a user supplied vtkPolyData.
               * \param[in] pd vtkPolyData raw pointer.
               *
               */
              virtual void Initialize(vtkSmartPointer<vtkPolyData> pd);

              /** \brief Initialize the skeleton with empty data.
               *
               */
              virtual void Initialize()
              { this->Initialize(nullptr); }

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

              /** \brief Sets the color of the representation.
               * \param[in] color QColor object.
               *
               */
              void setRepresentationColor(const QColor &color);

              /** \brief Returns the representation color.
               *
               */
              const QColor &representationColor() const
              { return m_color; }

              /** \brief Updates the representation.
               *
               */
              void UpdateRepresentation();

              /** \brief Returns true if the last event has resulted in data modification.
               *  Used to signal modifications to the parent tool.
               *
               */
              bool eventModifiedData() const
              { return m_modified; }

              /** \brief Deactivated the modified flag.
               *
               */
              void resetModifiedFlag()
              { m_modified = false; }

              /** \brief Operation modes. */
              enum { Start = 0, Define = 1, Manipulate = 2 };

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

              /** \brief Returns the number of nodes in the current representation.
               *
               */
              const unsigned int numberOfPoints() const;

            protected:
              int       m_widgetState;   /** widget operation state.                */
              Plane     m_orientation;   /** orthogonal plane of the widget.        */
              Nm        m_slice;         /** current slice position in Nm.          */
              Nm        m_shift;         /** actor's shift over the slice position. */
              QColor    m_color;         /** color of the representation.           */
              NmVector3 m_spacing;       /** representation's spacing.              */

              /** \brief Sets the current operation mode of the widget.
               * \param[in] mode integer of operation mode.
               *
               */
              void setCurrentOperationMode(const int mode);

              /** \brief Returns the integer of the current operation mode of the widget.
               *
               */
              const int currentOperationMode() const;

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

              QCursor m_crossMinusCursor; /** cross minus cursor shown when deleting nodes.                     */
              QCursor m_crossPlusCursor;  /** cross plus cursor shown when adding nodes between existing nodes. */
              QCursor m_crossCheckCursor; /** cross check cursor shown when connecting two strokes.             */
              bool    m_modified;         /** modified flag.                                                    */
          };

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_H_
