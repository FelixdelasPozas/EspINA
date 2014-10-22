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

#ifndef ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_
#define ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/NmVector3.h>

// VTK
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

// Qt
#include <QList>
#include <QMap>
#include <QColor>

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkGlyph3D;
class vtkPoints;
class vtkSphereSource;
class vtkUnsignedCharArray;

namespace ESPINA
{

  /** \class vtkSkeletonWidgetRepresentation.
   * Representation for the SkeletonWidget class.
   */
  class EspinaGUI_EXPORT vtkSkeletonWidgetRepresentation
  : public vtkWidgetRepresentation
  {
    private:
      /** \class SkeletonNode
       *  \brief To represent the skeleton as a list of nodes.
       *
       */
      class SkeletonNode
      {
        public:
          double worldPosition[3];
          QList<SkeletonNode *> connections;

          SkeletonNode(double worldPos[3])
          {
            worldPosition[0] = worldPos[0];
            worldPosition[1] = worldPos[1];
            worldPosition[2] = worldPos[2];
          }

          ~SkeletonNode()
          {
            for(auto node: connections)
              node->connections.removeAll(this);
          }
      };

    public:
      friend class vtkSkeletonWidget;

      /** \brief Implements vtkObject::New() for this class.
       *
       */
      static vtkSkeletonWidgetRepresentation* New();

      vtkTypeMacro(vtkSkeletonWidgetRepresentation,vtkWidgetRepresentation);

      /** \brief Overrides vtkWidgetRepresentation::PrintSelf().
       *
       */
      void PrintSelf(std::ostream& os, vtkIndent indent);

      /** \brief Adds a node to the skeleton at a given position.
       * \param[in] worldPos world coordinate vector.
       *
       */
      void AddNodeAtPosition(double worldPos[3]);

      /** \brief Adds a node to the skeleton at a given position.
       * \param[in] x x world coordinate.
       * \param[in] y y world coordinate.
       * \param[in] z z world coordinate.
       *
       */
      void AddNodeAtWorldPosition(double x, double y, double z);

      /** \brief Adds a node to the skeleton at a given position.This will be
       * converted into a world position according to the current slice.
       * \param[in] x x display coordinate.
       * \param[in] y y display coordinate.
       *
       */
      void AddNodeAtDisplayPosition(int x, int y);

      /** \brief Adds a node to the skeleton at a given position.This will be
       * converted into a world position according to the current slice.
       * \param[in] displayPos display coordinate vector.
       *
       */
      void AddNodeAtDisplayPosition(int displayPos[2]);

      /** \brief Given a display position, activate a node. The closest
       * node within tolerance will be activated. If a node is
       * activated, true will be returned, otherwise false will be
       * returned.
       * \param[in] x x display coordinate.
       * \param[in] y y display coordinate.
       *
       */
      bool ActivateNode(int X, int Y);

      /** \brief Given a display position, activate a node. The closest
       * node within tolerance will be activated. If a node is
       * activated, true will be returned, otherwise false will be
       * returned.
       * \param[in] displayPos display coordinate vector.
       *
       */
      bool ActivateNode(int displayPos[2]);

      /** \brief Sets the current node as the one given as parameter.
       * \param[in] node skeleton node raw pointer.
       *
       */
      bool ActivateNode(SkeletonNode *node);

      /** \brief Sets the current node as none.
       *
       */
      void DeactivateNode();

      /** \brief Checks for the proximity of another node not directly
       * connected to the current vertex (to join at the end of the
       * operation).
       *
       */
      bool TryToJoin(int X, int Y);

      /** \brief Move the active node to a specified display position.
       * Returns false if there is no active node of the node
       * couldn't be moved to the position. Returns true otherwise.
       * \param[in] displayPos display coordinate vector.
       *
       */
      bool SetActiveNodeToDisplayPosition(int displayPos[2], bool updateRepresentation = true);

      /** \brief Move the active node to a specified display position.
       * Returns false if there is no active node of the node
       * couldn't be moved to the position. Returns true otherwise.
       * \param[in] x x display coordinate.
       * \param[in] y y display coordinate.
       *
       */
      bool SetActiveNodeToDisplayPosition(int X, int Y, bool updateRepresentation = true);

      /** \brief Move the active node to a specified world position.
       * Returns false if there is no active node or the node
       * couldn't be moved to the position. Returns true otherwise.
       * \param[in] x x world coordinate.
       * \param[in] y y world coordinate.
       * \param[in] z z world coordinate.
       *
       */
      bool SetActiveNodeToWorldPosition(double x, double y, double z, bool updateRepresentation = true);

      /** \brief Move the active node to a specified world position.
       * Returns false if there is no active node or the node
       * couldn't be moved to the position. Returns true otherwise.
       * \param[in] worldPos world coordinate vector.
       * \param[in] updateRepresentation true to re-build the representation and false otherwise.
       *
       */
      bool SetActiveNodeToWorldPosition(double worldPos[3], bool updateRepresentation = true);

      /** \brief Returns true if there is a current node, and its position it's returned
       * in the parameter.
       * \param[out] doublePos world coordinates vector.
       *
       */
      bool GetActiveNodeWorldPosition(double worldPos[3]);

      /** \brief Deletes the current node. Returns true on success or
       *  false if the active node did not indicate a valid node.
       *
       */
      bool DeleteCurrentNode();

      /** \brief Deletes all nodes.
       *
       */
      virtual void ClearAllNodes();

      /** \brief Given a specific X, Y pixel location, add a new node
       * at this location. Returns true on success and false otherwise.
       * \param[in] X x display coordinate.
       * \param[in] Y y display coordinate.
       *
       */
      bool AddNodeOnContour(int X, int Y);

      /** \brief Returns the number of nodes in the representation.
       *
       */
      unsigned int GetNumberOfNodes() const;

      /** \brief Sets the representation orientation. If succeeds returns true
       * or false otherwise.
       * \param[in] plane orientation plane.
       *
       */
      bool SetOrientation(Plane plane);

      /** \brief Returns the representation orientation.
       *
       */
      Plane GetOrientation() const
      { return m_orientation; }

      /** \brief The tolerance to use when calculations are performed in
       *  world coordinates.
       *  \param[in] value tolerance value.
       *
       */
      void SetTolerance(const double value)
      { if(value < 0 || m_tolerance== value) return; m_tolerance = value; }

      /** \brief Sets the color of the representation.
       * \param[in] color QColor object.
       *
       */
      void SetColor(const QColor &color);

      /** \brief Returns the current tolerance for the widget.
       *
       */
      double GetTolerance()
      { return m_tolerance; }

      // States
      enum
      {
        Outside = 0, NearContour, NearPoint
      };

      /** \brief Implements vtkAbstractWidgetRepresentation::BuildRepresentation().
       *
       */
      void BuildRepresentation();

      /** \brief Updates the pointer position and visibility.
       *
       */
      void UpdatePointer();

      /* \brief Implements vtkAbstractWidgetRepresentation::ComputeInteractionState().
       *
       */
      virtual int ComputeInteractionState(int X, int Y, int vtkNotUsed(modified) = 0);

      /** \brief Implements vtkWidgetRepresentation::ReleaseGraphicResources().
       *
       */
      virtual void ReleaseGraphicsResources(vtkWindow *w);

      /** \brief Implements vtkWidgetRepresentation::RenderOverlay().
       *
       */
      virtual int RenderOverlay(vtkViewport *viewport);

      /** \brief Implements vtkWidgetRepresentation::RenderOpaqueGeometry().
       *
       */
      virtual int RenderOpaqueGeometry(vtkViewport *viewport);

      /** \brief Implements vtkWidgetRepresentation::RenderTranslucentGeometry().
       *
       */
      virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);

      /** \brief Implements vtkWidgetRepresentation::HasTranslucentPolygonalGeometry().
       *
       */
      virtual int HasTranslucentPolygonalGeometry();

      /** \brief Returns the current representation data as a vtkPolyData smart pointer.
       *
       */
      vtkSmartPointer<vtkPolyData> GetRepresentationPolyData() const;

      /** \brief Sets the slice of the representation.
       * \param[in] value slice value.
       *
       */
      void SetSlice(const Nm slice);

      /** \brief Sets the spacing in z needed to build the representation over the other representations.
       * \param[in] value spacing value in Z for the view.
       *
       */
      void SetShift(const Nm shift)
      { m_shift = shift; }

      /** \brief Sets the spacing of the representation to make the position of all nodes of the
       * representation centered on voxel center.
       * \param[in] spacing Spacing vector.
       *
       */
      void SetSpacing(const NmVector3 &spacing)
      { m_spacing = spacing; }

      /** \brief Returns the slice of the representation.
       *
       */
      double GetSlice() const
      { return m_slice; }

      /** \brief Returns true if the coordinates are near a node.
       * \param[in] x x display coordinate.
       * \param[in] y y display coordinate.
       *
       */
      bool IsNearNode(int x, int y) const;

      /** \brief Returns true if the point is too close to the previous point
       * using the tolerance value.
       * \param[in] x x display coordinate.
       * \param[in] y y display coordinate.
       *
       */
      bool IsPointTooClose(int x, int y) const;

    private:
      /** \brief vtkSkeletonWidgetRepresentation class private constructor.
       *
       */
      vtkSkeletonWidgetRepresentation();

      /** \brief vtkSkeletonWidgetRepresentation class virtual destructor.
       *
       */
      virtual ~vtkSkeletonWidgetRepresentation();

      /** \brief Given a display position this computes the world position
       * using the renderer of this class.
       * \param[in] displayPos display coordinate vector.
       * \param[out] worldPos world coordinate vector.
       *
       */
      void GetWorldPositionFromDisplayPosition(int displayPos[2], double worldPos[3]) const;

      /** \brief Returns the closes point on the skeleton to a given display coordinate.
       * \param[in] X x display coordinate.
       * \param[in] Y y display coordinate.
       * \param[out] worldPos world coordinate vector of the closest node.
       * \param[out] nodeIndex position index of the closest node in s_skeleton
       *
       */
      void FindClosestNode(int X, int Y, double worldPos[3], int &nodeIndex) const;

      /** \brief Build a skeleton representation from externally supplied PolyData.
       * \param[in] data vtkPolyData smart pointer.
       *
       */
      void Initialize(vtkSmartPointer<vtkPolyData> data);

      /** \brief Returns the shortest distance to the skeleton given a point in world coordinates.
       * \param[in] X x display coordinate.
       * \param[in] Y y display coordinate.
       * \param[out] worldPos world coordinate vector of the closest point to the skeleton.
       * \param[out] node_i position index of the i node in s_skeleton
       * \param[out] node_j position index of the j node in s_skeleton
       *
       */
      double FindClosestDistanceAndNode(int X, int Y, double worldPos[3], int &node_i, int &node_j) const;

      // Not implemented.
      vtkSkeletonWidgetRepresentation(const vtkSkeletonWidgetRepresentation&);
      void operator=(const vtkSkeletonWidgetRepresentation&);

    protected:
      static const double s_sliceWindow;

      Plane     m_orientation;
      double    m_tolerance;
      Nm        m_slice;
      Nm        m_shift;
      QColor    m_color;
      NmVector3 m_spacing;

      static QList<SkeletonNode *> s_skeleton;
      static SkeletonNode         *s_currentVertex;

      QMap<SkeletonNode *, vtkIdType> m_visiblePoints;

      // Support picking
      double m_lastPickPosition[2];
      double m_lastEventPosition[2];
      double m_interactionOffset[2]; // distance between the mouse event and where the widget is focused (distance to maintain between interaction).

      // VTK data;
      vtkSmartPointer<vtkUnsignedCharArray> m_colors;
      vtkSmartPointer<vtkGlyph3D>           m_pointer;
      vtkSmartPointer<vtkActor>             m_pointerActor;
      vtkSmartPointer<vtkPoints>            m_points;
      vtkSmartPointer<vtkPolyData>          m_pointsData;
      vtkSmartPointer<vtkGlyph3D>           m_glypher;
      vtkSmartPointer<vtkPolyDataMapper>    m_mapper;
      vtkSmartPointer<vtkActor>             m_actor;
      vtkSmartPointer<vtkPolyData>          m_lines;
      vtkSmartPointer<vtkPolyDataMapper>    m_linesMapper;
      vtkSmartPointer<vtkActor>             m_linesActor;
  };

} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_
