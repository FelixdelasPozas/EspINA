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

using namespace std;

namespace ESPINA
{
  class SkeletonWidget;

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

      /** \brief Returns the orientation of the widget.
       *
       */
      virtual Plane GetOrientation();

      /** \brief Sets the parent SkeletonWidget for this vtk widget.
       * \param[in] parent SkeletonWidget raw pointer.
       *
       * Parent needed to signal start/end of a contour.
       *
       */
      void setParentWidget(SkeletonWidget *parent)
      { m_parent = parent; }

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

      /** \brief Sets the tolerance value of the widget.
       * \param[in] tolerance tolerance value.
       *
       */
      void SetTolerance(const double tolerance);

      /** \brief Sets the spacing of the view in the Z coordinate to draw the representation correctly.
       *
       */
      void SetShift(const Nm spacing);

      /** \brief Sets the spacing of the representation to make the position of all nodes of the
       * representation centered on voxel center.
       * \param[in] spacing Spacing vector.
       *
       */
      void SetSpacing(const NmVector3 &spacing);

      /** \brief Sets the color of the representation.
       * \param[in] color QColor object.
       *
       */
      void setRepresentationColor(const QColor &color);

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

      enum
      {
        Start, Define, Manipulate
      };

    protected:
      int       m_widgetState;
      int       m_currentHandle;
      Plane     m_orientation;
      double    m_drawTolerance;
      Nm        m_slice;
      Nm        m_shift;
      QColor    m_color;
      NmVector3 m_spacing;

      /** \brief Callback interface to capture events when placing the widget.
       *
       */
      static void StopAction(vtkAbstractWidget*);
      static void MoveAction(vtkAbstractWidget*);
      static void KeyPressAction(vtkAbstractWidget *);
      static void ReleaseKeyPressAction(vtkAbstractWidget *);
      static void TranslateAction(vtkAbstractWidget *);

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
      vtkSkeletonWidget(const vtkSkeletonWidget&);

      /** \brief Assignment operator not implemented.
       *
       */
      void operator=(const vtkSkeletonWidget&);

      QCursor         m_crossMinusCursor, m_crossPlusCursor, m_crossCheckCursor;
      SkeletonWidget *m_parent;
      bool            m_modified;
  };

} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_H_
