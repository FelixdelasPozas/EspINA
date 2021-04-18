/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_VTK_ORTHOGONAL_REGION_SLICE_REPRESENTATION_H
#define ESPINA_VTK_ORTHOGONAL_REGION_SLICE_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkSmartPointer.h>
#include "vtkWidgetRepresentation.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkCellPicker;
class vtkProperty;
class vtkPolyData;
class vtkPoints;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class EspinaGUI_EXPORT vtkOrthogonalRepresentation2D
          : public vtkWidgetRepresentation
          {
            public:
              static vtkOrthogonalRepresentation2D *New();

              vtkTypeMacro(vtkOrthogonalRepresentation2D,vtkWidgetRepresentation);
              void PrintSelf(ostream& os, vtkIndent indent);

              /** \brief Sets the representation depth in the plane it is visible.
               * \param[in] depth Depth in Nm.
               *
               */
              void SetDepth(const double depth);

              /** \brief Sets the plane of the view the representation will be shown.
               * \param[in] plane Plane enum value.
               *
               */
              void SetPlane(const Plane plane);

              /** \brief Sets the slice of the representation in the planar view.
               * \param[in] pos Position in Nm.
               *
               */
              void SetSlice(Nm pos);

              /** \brief Sets the bounds of the representation.
               * \param[in] bounds Bounds in Nm.
               *
               */
              void SetOrthogonalBounds(double bounds[6]);

              /** \brief Returns the representation bounds in the given array.
               * \param[out] bounds Bounds in Nm.
               *
               */
              void GetOrthogonalBounds(double bounds[6]);

              virtual void PlaceWidget(double bounds[6]);
              virtual void BuildRepresentation();
              virtual int  ComputeInteractionState(int X, int Y, int modify=0);
              virtual void StartWidgetInteraction(double e[2]);
              virtual void WidgetInteraction(double e[2]);
              virtual double *GetBounds();

              virtual void ReleaseGraphicsResources(vtkWindow*);
              virtual int  RenderOpaqueGeometry(vtkViewport*);
              virtual int  RenderTranslucentPolygonalGeometry(vtkViewport*);
              virtual int  HasTranslucentPolygonalGeometry();


              //BTX - used to manage the state of the widget
              enum {Outside=0, Inside,
                    MoveLeft,  MoveRight,
                    MoveTop,   MoveBottom,
                    Translating
              };

              //ETX

              /** \brief The interaction state may be set from a widget (e.g., vtkBoxWidget2) or
               * other object. This controls how the interaction with the widget
               * proceeds. Normally this method is used as part of a handshaking
               * process with the widget: First ComputeInteractionState() is invoked that
               * returns a state based on geometric considerations (i.e., cursor near a
               * widget feature), then based on events, the widget may modify this
               * further.
               * \param[in] state Representation state value in state enum.
               */
              void SetInteractionState(const int state);

              /** \brief Sets the representation color.
               * \param[in] color Color rgb values array.
               *
               */
              void setRepresentationColor(const double *color);

              /** \brief Sets the representation pattern.
               * \param[in] patter Pattern value.
               *
               */
              void setRepresentationPattern(const int pattern);

            protected:
              /** \brief vtkOrthogonalRepresentation2D class constructor.
               *
               */
              vtkOrthogonalRepresentation2D();

              /** \brief vtkOrthogonalRepresentation2D class virtual destructor.
               *
               */
              virtual ~vtkOrthogonalRepresentation2D();

              double LastEventPosition[3]; /** position of the last event. */

              // Counting Region Edge
              vtkSmartPointer<vtkActor>          EdgeActor[4];    /** edges actors.           */
              vtkSmartPointer<vtkPolyDataMapper> EdgeMapper[4];   /** edges polydata mappers. */
              vtkSmartPointer<vtkPolyData>       EdgePolyData[4]; /** edges polydata.         */
              vtkSmartPointer<vtkPoints>         Vertex;          /** representation points.  */

              /** \brief Changes the property of the given actor to highlight it.
               * \param[in] actor Edge actor to modify.
               *
               */
              void HighlightEdge(vtkSmartPointer<vtkActor> actor);

              /** \brief Highlights all edges.
               *
               */
              void Highlight();

              // Do the picking
              vtkSmartPointer<vtkCellPicker> EdgePicker;  /** edge picker.                             */
              vtkSmartPointer<vtkCellPicker> LastPicker;  /** edge picker of last interaction or null. */
              vtkSmartPointer<vtkActor>      CurrentEdge; /** edge actor of the current interaction.   */

              vtkSmartPointer<vtkProperty> EdgeProperty;         /** edge actor property.                  */
              vtkSmartPointer<vtkProperty> SelectedEdgeProperty; /** edge actor property when selected.    */
              vtkSmartPointer<vtkProperty> InvisibleProperty;    /** edge actor property when not visible. */

              /** \brief Helper method to create the edge actors properties.
               *
               */
              virtual void CreateDefaultProperties();

            private:
              /** \brief Helper method that returns the index of the horizontal coordinate in the view.
               *
               */
              const int hCoord() const {return Plane::YZ == m_plane?2:0;}

              /** \brief Helper method that returns the index of the vertical coordinate in the view.
               *
               */
              const int vCoord() const {return Plane::XZ == m_plane?2:1;}

              /** \brief Helper method to create the orthogonal region.
               *
               */
              void CreateRegion();

              /** \brief Updates the region representation.
               *
               */
              void UpdateRegion();

              /** \brief Updates the region representation for a XY planar view.
               *
               */
              void UpdateXYFace();

              /** \brief Updates the region representation for a YZ planar view.
               *
               */
              void UpdateYZFace();

              /** \brief Updates the region representation for a XZ planar view.
               *
               */
              void UpdateXZFace();

              /** \brief Helper method to move the left edge.
               * \param[in] from Initial point of the movement.
               * \param[in] to Final point of the movement.
               *
               */
              void MoveLeftEdge(double *from, double *to);

              /** \brief Helper method to move the right edge.
               * \param[in] from Initial point of the movement.
               * \param[in] to Final point of the movement.
               *
               */
              void MoveRightEdge(double *from, double *to);

              /** \brief Helper method to move the top edge.
               * \param[in] from Initial point of the movement.
               * \param[in] to Final point of the movement.
               *
               */
              void MoveTopEdge(double *from, double *to);

              /** \brief Helper method to move the bottom edge.
               * \param[in] from Initial point of the movement.
               * \param[in] to Final point of the movement.
               *
               */
              void MoveBottomEdge(double *from, double *to);

              /** \brief Helper method to translate all edges.
               * \param[in] from Initial point of the movement.
               * \param[in] to Final point of the movement.
               *
               */
              void Translate(double *from, double *to);

              /** \brief Returns the depth of the representation in the current view.
               *
               */
              const double sliceDepth() const;

              /** \brief Updates the color of the given property.
               * \param[in] property Edge actor property.
               *
               */
              void updateEdgeColor  (vtkProperty *property);

              /** \brief Updates the pattern of the given property.
               * \param[in] property Edge actor property.
               *
               */
              void updateEdgePattern(vtkProperty *property);

            private:
              /** Edge enum */
              enum class Edge: char {LEFT = 0, TOP, RIGHT, BOTTOM};

              vtkOrthogonalRepresentation2D(const vtkOrthogonalRepresentation2D&);  // copy constructor not implemented
              void operator=(const vtkOrthogonalRepresentation2D&);  // assign operator not implemented

              Plane  m_plane;        /** plane of the representation.                     */
              Nm     m_slice;        /** current slice of the representation.             */
              double m_bounds[6];    /** representation bounds in 3D.                     */
              double m_repBounds[6]; /** bounds of the representation in the planar view. */
              double LeftEdge;       /** left edge position.                              */
              double TopEdge;        /** top edge position.                               */
              double RightEdge;      /** right edge position.                             */
              double BottomEdge;     /** bottom edge position.                            */
              double m_depth;        /** depth of the representation in the planar view.  */
              double m_color[3];     /** rgb values of the color of the representation.   */
              int    m_pattern;      /** pattern value of the representation.             */
          };
        }
      }
    }
  }

} // namespace ESPINA

#endif
