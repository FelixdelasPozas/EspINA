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
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Utils/Vector3.hxx>
#include <vtkSetGet.h>
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

// Qt
#include <QList>
#include <QMap>
#include <QColor>
#include <QMutex>

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkGlyph3D;
class vtkPoints;
class vtkSphereSource;
class vtkUnsignedCharArray;
class vtkStringArray;
class vtkTextProperty;
class vtkPointSetToLabelHierarchy;
class vtkLabelPlacementMapper;
class vtkActor2D;
class vtkTransformPolyDataFilter;
class vtkGlyphSource2D;
class vtkFollower;
class vtkGlyph3DMapper;

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
          /** \class vtkSkeletonWidgetRepresentation.
           * \brief VTK Representation for the SkeletonWidget class.
           *
           */
          class EspinaGUI_EXPORT vtkSkeletonWidgetRepresentation
          : public vtkWidgetRepresentation
          {
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
              bool ActivateNode(Core::SkeletonNode *node);

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

              /** \brief Moves the active node to a specified display position.
               * Returns false if there is no active node of the node
               * couldn't be moved to the position. Returns true otherwise.
               * \param[in] displayPos display coordinate vector.
               *
               */
              bool SetActiveNodeToDisplayPosition(int displayPos[2], bool updateRepresentation = true);

              /** \brief Moves the active node to a specified display position.
               * Returns false if there is no active node of the node
               * couldn't be moved to the position. Returns true otherwise.
               * \param[in] x x display coordinate.
               * \param[in] y y display coordinate.
               *
               */
              bool SetActiveNodeToDisplayPosition(int X, int Y, bool updateRepresentation = true);

              /** \brief Moves the active node to a specified world position.
               * Returns false if there is no active node or the node
               * couldn't be moved to the position. Returns true otherwise.
               * \param[in] x x world coordinate.
               * \param[in] y y world coordinate.
               * \param[in] z z world coordinate.
               *
               */
              bool SetActiveNodeToWorldPosition(double x, double y, double z, bool updateRepresentation = true);

              /** \brief Moves the active node to a specified world position.
               * Returns false if there is no active node or the node
               * couldn't be moved to the position. Returns true otherwise.
               * \param[in] worldPos world coordinate vector.
               * \param[in] updateRepresentation true to re-build the representation and false otherwise.
               *
               */
              bool SetActiveNodeToWorldPosition(double worldPos[3], bool updateRepresentation = true);

              /** \brief Selects and sets the last node as active node, if any.
               * \param[in] updateRepresentation true to re-build the representation and false otherwise.
               *
               */
              bool SetActiveNodeToLastNode(bool updateRepresentation = false);

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

              /** \brief Inserts the stroke if not present in the list or updates the properties of the stroke.
               * \param[in] stroke SkeletonStroke struct.
               *
               */
              void setStroke(const Core::SkeletonStroke &stroke);

              /** \brief Removes the stroke from the stroke list and all its related edges.
               * \param[in] stroke SkeletonStroke struct.
               *
               */
              void removeStroke(const Core::SkeletonStroke &stroke);

              /** \brief Renames the stroke with the old name to the new one.
               * \param[in] oldName Stroke current name.
               * \param[in] newName Stroke new name.
               *
               */
              void renameStroke(const QString &oldName, const QString &newName);

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

              void BuildRepresentation();

              /** \brief Updates the pointer position and visibility.
               *
               */
              void UpdatePointer();

              virtual int ComputeInteractionState(int X, int Y, int vtkNotUsed(modified) = 0);

              virtual void ReleaseGraphicsResources(vtkWindow *w);

              virtual int RenderOverlay(vtkViewport *viewport);

              virtual int RenderOpaqueGeometry(vtkViewport *viewport);

              virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);

              virtual int HasTranslucentPolygonalGeometry();

              /** \brief Returns the current representation data as a vtkPolyData smart pointer.
               *
               */
              vtkSmartPointer<vtkPolyData> GetRepresentationPolyData();

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
              void SetSpacing(const NmVector3 &spacing);

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

              /** \brief Sets the value of the ignore cursor flag.
               * \param[in] value true to ignore the cursor node in 'near node' and state computations, false otherwise.
               *
               */
              void setIgnoreCursorNode(bool value)
              { m_ignoreCursor = value; }

              /** \brief Returns the value of the ignore cursor node flag.
               *
               */
              bool ignoresCursor() const
              { return m_ignoreCursor; }

              /** \brief Sets the color of the representation labels.
               * \param[in] color QColor object.
               *
               */
              void setLabelsColor(const QColor &color);

              /** \brief Returns the color of the representation labels.
               *
               */
              const QColor &labelsColor() const
              { return m_labelColor; }

              /** \brief Sets the text size of the representations labels.
               * \param[in] size Text size in (3-15)
               *
               */
              void setLabelsSize(unsigned int size);

              /** \brief Returns the text size of the labels.
               *
               */
              const unsigned int labelsTextSize() const
              { return m_labelSize; }

              /** \brief Enables/disables showing the labels.
               * \param[in] value True to make labels visible and false otherwise.
               *
               */
              void setShowLabels(const bool value);

              /** \brief Returns true if the representations shows the skeleton labels and false otherwise.
               *
               */
              const bool showLabels() const
              { return m_showLabels; }

              /** \brief Sets the width of the skeleton lines.
               *
               */
              void setWidth(const int width);

              /** \brief Returns the width of the skeleton representation.
               *
               */
              const int width() const
              { return m_width; }

              /** \brief Removes all nodes from the static skeleton representation.
               *
               */
              static void ClearRepresentation();

              /** \brief Build a skeleton representation from externally supplied PolyData.
               * \param[in] data vtkPolyData smart pointer.
               *
               */
              static void Initialize(vtkSmartPointer<vtkPolyData> data = nullptr);

              /** \brief Using the last three points connects the first and last with the current stroke and creates
               *  a connection using the given stroke, the second point and the closest point to the first-last segment.
               *  Returns true on success and false if a connection cannot be created (less than three nodes in the skeleton).
               *  \param[in] stroke Stroke of the connection.
               *
               */
              bool createConnection(const Core::SkeletonStroke &stroke);

              /** \brief Performs a stroke change after adding a point to the skeleton.
               *  \param[in] stroke Stroke to change to.
               *
               */
              bool switchToStroke(const Core::SkeletonStroke &stroke);

              /** \brief Returns true if the given point does not collide with a stroke or, if it collides, cannot be considered
               * a continuation.
               * \param[in] point Point 3D coordinates.
               *
               */
              bool isStartNode(const NmVector3 &point);

              /** \brief Adds a property to the stroke of the current node or removes it if present.
               * \param[in] property Node property to toggle in the stroke.
               * \param[in] x Event 2d x coordinate reference.
               * \param[in] y Event 2d y coordinate reference.
               *
               */
              bool ToggleStrokeProperty(const Core::SkeletonNodeProperty property, const int &x, const int &y);

              /** \brief Sets that strokes with the same color must change colors to facilitate visualization.
               * \param[in] value True to change hue of same colored strokes and false to show strokes with its defined hue.
               *
               */
              void setChangeCoincidetHue(const bool value);

              /** \brief Returns true if the color of strokes that are coincident in hue is changed to facilitate visualization.
               * Returns false otherwise.
               *
               */
              const bool changeCoincidentHue() const;
			  
              /** \brief Sets the representation default HUE value.
               * \param[in] value Hue value in (0-359).
               *
               */
              void setDefaultHueValue(const int value)
              { m_defaultHue = value % 360; }

            private:
              /** \brief vtkSkeletonWidgetRepresentation class private constructor.
               *
               */
              vtkSkeletonWidgetRepresentation();

              /** \brief vtkSkeletonWidgetRepresentation class virtual destructor.
               *
               */
              virtual ~vtkSkeletonWidgetRepresentation()
              {}

              /** \brief Given a display position this computes the world position
               * using the renderer of this class.
               * \param[in] displayPos Display coordinates.
               * \param[out] worldPos World coordinates.
               *
               */
              void GetWorldPositionFromDisplayPosition(const int displayPos[2], double worldPos[3]) const;

              /** \brief Given a world position this computes the pixel coordinates in display coordinates.
               * \param[in] worldPos World coordinates.
               * \param[out] displayPos Display coordinates.
               *
               */
              void GetDisplayPositionFromWorldPosition(const double worldPos[3], int displayPos[2]) const;

              /** \brief Returns the closes point on the skeleton to a given display coordinate.
               * \param[in] X x display coordinate.
               * \param[in] Y y display coordinate.
               * \param[out] worldPos world coordinate vector of the closest node.
               * \param[out] nodeIndex position index of the closest node in s_skeleton
               *
               */
              void FindClosestNode(const int X, const int Y, double worldPos[3], int &nodeIndex) const;

              /** \brief Returns the shortest distance to the skeleton given a point in world coordinates.
               * \param[in] X x display coordinate.
               * \param[in] Y y display coordinate.
               * \param[out] worldPos world coordinate vector of the closest point to the skeleton.
               * \param[out] node_i position index of the i node in s_skeleton
               * \param[out] node_j position index of the j node in s_skeleton
               *
               */
              double FindClosestDistanceAndNode(const int X, const int Y, double worldPos[3], int &node_i, int &node_j) const;

              /** \brief Merges paths whose common node is terminal without the connection to the other path.
               *
               */
              void performPathsMerge();

              /** \brief Replaces some Spine strokes with SubSpine if necessary.
               *
               */
              void performSpineSplitting();

              /** \brief Helper method to obtain the paths of a given node.
               * \param[in] node Skeleton node.
               *
               */
              const Core::PathList pathsOfNode(Core::SkeletonNode *node) const;

              /** \brief Helper method that returns the paths of the current selected node.
               * \param[in] x View 2d event x coordinate reference.
               * \param[in] y View 2d event y coordinate reference.
               *
               */
              const Core::PathList currentSelectedPaths(const int &x, const int &y) const;

              /** \brief Deletes the given node and the connected ones if not connected to anyone else.
               * \param[in] node Skeleton node struct.
               *
               */
              void deleteNode(Core::SkeletonNode *node);

              // Not implemented.
              vtkSkeletonWidgetRepresentation(const vtkSkeletonWidgetRepresentation&);
              void operator=(const vtkSkeletonWidgetRepresentation&);

            protected:
              /** \brief Helper method that returns the current stroke definition.
               *
               */
              Core::SkeletonStroke currentStroke() const;

              /** \brief Helper method to compute a very close but different color than the one given
               * for differentiating close strokes. It looks in the number of strokes with the same hue
               * and computes a new QColor based on number of coincidences.
               * \param[in] stroke Skeleton stroke to compute color.
               */
              const QColor computeCoincidentStrokeColor(const Core::SkeletonStroke &stroke);

              Plane     m_orientation;
              double    m_tolerance;
              Nm        m_slice;
              Nm        m_shift;
              NmVector3 m_spacing;
			  int       m_defaultHue;

              static Core::SkeletonDefinition s_skeleton;
              static NmVector3                s_skeletonSpacing;
              static Core::SkeletonNode      *s_currentVertex;
              static QMutex                   s_skeletonMutex;

              QMap<Core::SkeletonNode *, vtkIdType> m_visiblePoints;

              // Support picking
              double m_lastPickPosition[2];
              double m_lastEventPosition[2];
              double m_interactionOffset[2]; // distance between the mouse event and where the widget is focused (distance to maintain between interaction).

              // VTK data;
              vtkSmartPointer<vtkUnsignedCharArray>        m_colors;            /** node colors.                                   */
              vtkSmartPointer<vtkGlyph3D>                  m_pointer;           /** current node glyph.                            */
              vtkSmartPointer<vtkActor>                    m_pointerActor;      /** current node actor.                            */
              vtkSmartPointer<vtkPoints>                   m_points;            /** node locations.                                */
              vtkSmartPointer<vtkPolyData>                 m_pointsData;        /** points data.                                   */
              vtkSmartPointer<vtkGlyph3D>                  m_glypher;           /** nodes glyph                                    */
              vtkSmartPointer<vtkPolyDataMapper>           m_mapper;            /** nodes mapper.                                  */
              vtkSmartPointer<vtkActor>                    m_actor;             /** nodes actor.                                   */
              vtkSmartPointer<vtkPolyData>                 m_lines;             /** solid lines data.                              */
              vtkSmartPointer<vtkPolyDataMapper>           m_linesMapper;       /** solid lines mapper.                            */
              vtkSmartPointer<vtkActor>                    m_linesActor;        /** solid lines actor.                             */
              vtkSmartPointer<vtkPolyData>                 m_dashedLines;       /** dashed lines data.                             */
              vtkSmartPointer<vtkPolyDataMapper>           m_dashedLinesMapper; /** dashed lines mapper.                           */
              vtkSmartPointer<vtkActor>                    m_dashedLinesActor;  /** dashed lines actor.                            */

              vtkSmartPointer<vtkPoints>                   m_truncatedPoints;   /** truncated points.                              */
              vtkSmartPointer<vtkPolyData>                 m_truncatedData;     /** truncated polydata.                            */
              vtkSmartPointer<vtkTransformPolyDataFilter>  m_transformFilter;   /** transformed polydata filter for XZ & YZ views. */
              vtkSmartPointer<vtkGlyph3DMapper>            m_glyphMapper;       /** glyph mapper filter.                           */
              vtkSmartPointer<vtkGlyphSource2D>            m_glyph2D;           /** glyph source for 2D views.                     */
              vtkSmartPointer<vtkFollower>                 m_truncatedActor;    /** truncated points representation actor.         */

              bool                                         m_showLabels;        /** true to show labels actor, false otherwise.    */
              vtkSmartPointer<vtkPoints>                   m_labelPoints;       /** points where to show the labels.               */
              vtkSmartPointer<vtkStringArray>              m_labels;            /** label points text.                             */
              vtkSmartPointer<vtkPolyData>                 m_labelData;         /** polydata with the points and text array.       */
              vtkSmartPointer<vtkTextProperty>             m_labelProperty;     /** label visual properties.                       */
              vtkSmartPointer<vtkPointSetToLabelHierarchy> m_labelFilter;       /** points to labels filter                        */
              vtkSmartPointer<vtkLabelPlacementMapper>     m_labelPlacer;       /** labels mapper.                                 */
              vtkSmartPointer<vtkActor2D>                  m_labelActor;        /** labels actor.                                  */
              QColor                                       m_labelColor;        /** color of the labels.                           */
              unsigned int                                 m_labelSize;         /** labels text size.                              */
              int                                          m_width;             /** skeleton lines width.                          */

              int m_currentStrokeIndex; /** index of current stroke in s_skeleton.strokes. */
              int m_currentEdgeIndex;   /** index of current edge in s_skeleton.edges.     */

              bool m_changeCoincidentHue; /** true to change the hue of strokes with the same color to facilitate visualization. */

            private:
              bool m_ignoreCursor; /** flag, true to ignore the cursor node (if any) in some computations, false otherwise. */
          };

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_
