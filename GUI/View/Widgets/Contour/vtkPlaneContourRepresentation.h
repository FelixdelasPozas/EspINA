/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef _VTKPLANECONTOURREPRESENTATION_H_
#define _VTKPLANECONTOURREPRESENTATION_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkContourRepresentation.h>
#include <vtkSmartPointer.h>

class vtkContourLineInterpolator;
class vtkIncrementalOctreePointLocator;
class vtkPointPlacer;
class vtkPolyData;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Contour
        {

          //----------------------------------------------------------------------
          //BTX
          class vtkPlaneContourRepresentationPoint
          {
          public:
            double WorldPosition[3];
            double NormalizedDisplayPosition[2];
          };

          class vtkPlaneContourRepresentationNode
          {
          public:
            double WorldPosition[3];
            double WorldOrientation[9];
            double NormalizedDisplayPosition[2];
            int Selected;
            std::vector<vtkPlaneContourRepresentationPoint*> Points;
          };

          class vtkPlaneContourRepresentationInternals
          {
          public:
            std::vector<vtkPlaneContourRepresentationNode*> Nodes;

            void
            ClearNodes()
            {
              for (unsigned int i = 0; i < this->Nodes.size(); i++)
              {
                for (unsigned int j = 0; j < this->Nodes[i]->Points.size(); j++)
                  delete this->Nodes[i]->Points[j];

                this->Nodes[i]->Points.clear();
                delete this->Nodes[i];
              }
              this->Nodes.clear();
            }
          };
          //ETX

          class EspinaGUI_EXPORT vtkPlaneContourRepresentation
          : public vtkContourRepresentation
          {
            //BTX
            friend class vtkPlaneContourWidget;
            //ETX
          public:
            // Description:
            // Standard VTK methods.
            vtkTypeMacro(vtkPlaneContourRepresentation,vtkWidgetRepresentation);
            void PrintSelf(ostream& os, vtkIndent indent);

            // Description:
            // Add a node at a specific world position. Returns 0 if the
            // node could not be added, 1 otherwise.
            virtual int AddNodeAtWorldPosition(double x, double y, double z);
            virtual int AddNodeAtWorldPosition(double worldPos[3]);
            virtual int AddNodeAtWorldPosition(double worldPos[3], double worldOrient[9]);

            // Description:
            // Add a node at a specific display position. This will be
            // converted into a world position according to the current
            // constraints of the point placer. Return 0 if a point could
            // not be added, 1 otherwise.
            virtual int AddNodeAtDisplayPosition(double displayPos[2]);
            virtual int AddNodeAtDisplayPosition(int displayPos[2]);
            virtual int AddNodeAtDisplayPosition(int X, int Y);

            // Description:
            // Given a display position, activate a node. The closest
            // node within tolerance will be activated. If a node is
            // activated, 1 will be returned, otherwise 0 will be
            // returned.
            virtual int ActivateNode(double displayPos[2]);
            virtual int ActivateNode(int displayPos[2]);
            virtual int ActivateNode(int X, int Y);

            // Descirption:
            // Move the active node to a specified world position.
            // Will return 0 if there is no active node or the node
            // could not be moved to that position. 1 will be returned
            // on success.
            virtual int SetActiveNodeToWorldPosition(double pos[3]);
            virtual int SetActiveNodeToWorldPosition(double pos[3], double orient[9]);

            // Description:
            // Move the active node based on a specified display position.
            // The display position will be converted into a world
            // position. If the new position is not valid or there is
            // no active node, a 0 will be returned. Otherwise, on
            // success a 1 will be returned.
            virtual int SetActiveNodeToDisplayPosition(double pos[2]);
            virtual int SetActiveNodeToDisplayPosition(int pos[2]);
            virtual int SetActiveNodeToDisplayPosition(int X, int Y);

            // Description:
            // Set/Get whether the active or nth node is selected.
            virtual int ToggleActiveNodeSelected();
            virtual int GetActiveNodeSelected();
            virtual int GetNthNodeSelected(int);
            virtual int SetNthNodeSelected(int);

            // Description:
            // Get the world position of the active node. Will return
            // 0 if there is no active node, or 1 otherwise.
            virtual int GetActiveNodeWorldPosition(double pos[3]);

            // Description:
            // Get the world orientation of the active node. Will return
            // 0 if there is no active node, or 1 otherwise.
            virtual int GetActiveNodeWorldOrientation(double orient[9]);

            // Description:
            // Get the display position of the active node. Will return
            // 0 if there is no active node, or 1 otherwise.
            virtual int GetActiveNodeDisplayPosition(double pos[2]);

            // Description:
            // Get the number of nodes.
            virtual int GetNumberOfNodes();

            // Description:
            // Get the nth node's display position. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting).
            virtual int GetNthNodeDisplayPosition(int n, double pos[2]);

            // Description:
            // Get the nth node's world position. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting).
            virtual int GetNthNodeWorldPosition(int n, double pos[3]);

            // Description:
            // Get the nth node's world orientation. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting).
            virtual int GetNthNodeWorldOrientation(int n, double orient[9]);

            // Description:
            // Set the nth node's display position. Display position
            // will be converted into world position according to the
            // constraints of the point placer. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting) or the world position
            // is not valid.
            virtual int SetNthNodeDisplayPosition(int n, int X, int Y);
            virtual int SetNthNodeDisplayPosition(int n, int pos[2]);
            virtual int SetNthNodeDisplayPosition(int n, double pos[2]);

            // Description:
            // Set the nth node's world position. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting) or the world
            // position is not valid according to the point
            // placer.
            virtual int SetNthNodeWorldPosition(int n, double pos[3]);
            virtual int SetNthNodeWorldPosition(int n, double pos[3], double orient[9]);

            // Description:
            // Get the nth node's slope. Will return
            // 1 on success, or 0 if there are not at least
            // (n+1) nodes (0 based counting).
            virtual int GetNthNodeSlope(int idx, double slope[3]);

            // Descirption:
            // For a given node n, get the number of intermediate
            // points between this node and the node at
            // (n+1). If n is the last node and the loop is
            // closed, this is the number of intermediate points
            // between node n and node 0. 0 is returned if n is
            // out of range.
            virtual int GetNumberOfIntermediatePoints(int n);

            // Description:
            // Get the world position of the intermediate point at
            // index idx between nodes n and (n+1) (or n and 0 if
            // n is the last node and the loop is closed). Returns
            // 1 on success or 0 if n or idx are out of range.
            virtual int GetIntermediatePointWorldPosition(int n, int idx, double point[3]);

            // Description:
            // Add an intermediate point between node n and n+1
            // (or n and 0 if n is the last node and the loop is closed).
            // Returns 1 on success or 0 if n is out of range.
            virtual int AddIntermediatePointWorldPosition(int n, double point[3]);

            // Description:
            // Delete the last node. Returns 1 on success or 0 if
            // there were not any nodes.
            virtual int DeleteLastNode();

            // Description:
            // Delete the active node. Returns 1 on success or 0 if
            // the active node did not indicate a valid node.
            virtual int DeleteActiveNode();

            // Description:
            // Delete the nth node. Return 1 on success or 0 if n
            // is out of range.
            virtual int DeleteNthNode(int n);

            // Description:
            // Delete all nodes.
            virtual void ClearAllNodes();

            // Description:
            // Given a specific X, Y pixel location, add a new node
            // on the contour at this location.
            virtual int AddNodeOnContour(int X, int Y);

            // Description:
            // The tolerance to use when calculations are performed in
            // display coordinates
            vtkSetClampMacro(PixelTolerance,int,1,100);
            vtkGetMacro(PixelTolerance,int);

            // Description:
            // The tolerance to use when calculations are performed in
            // world coordinates
            vtkSetClampMacro(WorldTolerance, double, 0.0, VTK_DOUBLE_MAX);
            vtkGetMacro(WorldTolerance, double);

            //BTX -- used to communicate about the state of the representation
            enum
            {
              Outside = 0, Nearby, Inside, NearContour, NearPoint
            };

            enum
            {
              Inactive = 0, Translate, Shift, Scale
            };
            //ETX

            // Description:
            // Set / get the current operation. The widget is either
            // inactive, or it is being translated.
            vtkGetMacro(CurrentOperation, int);
            vtkSetClampMacro(CurrentOperation, int,vtkPlaneContourRepresentation::Inactive,vtkPlaneContourRepresentation::Scale );

            void SetCurrentOperationToInactive()
            {
              this->SetCurrentOperation(vtkPlaneContourRepresentation::Inactive);
            }

            void SetCurrentOperationToTranslate()
            {
              this->SetCurrentOperation(vtkPlaneContourRepresentation::Translate);
            }

            void SetCurrentOperationToShift()
            {
              this->SetCurrentOperation(vtkPlaneContourRepresentation::Shift);
            }

            void SetCurrentOperationToScale()
            {
              this->SetCurrentOperation(vtkPlaneContourRepresentation::Scale);
            }

            // Description:
            // Set / get the Point Placer. The point placer is
            // responsible for converting display coordinates into
            // world coordinates according to some constraints, and
            // for validating world positions.
            void SetPointPlacer(vtkPointPlacer *);vtkGetObjectMacro( PointPlacer, vtkPointPlacer );

            // Description:
            // Set / Get the Line Interpolator. The line interpolator
            // is responsible for generating the line segments connecting
            // nodes.
            void SetLineInterpolator(vtkContourLineInterpolator *);vtkGetObjectMacro( LineInterpolator, vtkContourLineInterpolator );

            // Description:
            // These are methods that satisfy vtkWidgetRepresentation's API.
            virtual void BuildRepresentation()=0;
            virtual int  ComputeInteractionState(int X, int Y, int modified = 0)=0;
            virtual void StartWidgetInteraction(double e[2])=0;
            virtual void WidgetInteraction(double e[2])=0;

            // Description:
            // Methods required by vtkProp superclass.
            virtual void ReleaseGraphicsResources(vtkWindow *w)=0;
            virtual int RenderOverlay(vtkViewport *viewport)=0;
            virtual int RenderOpaqueGeometry(vtkViewport *viewport)=0;
            virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport)=0;
            virtual int HasTranslucentPolygonalGeometry()=0;

            // Description:
            // Set / Get the ClosedLoop value. This ivar indicates whether the contour
            // forms a closed loop.
            void SetClosedLoop(int val);vtkGetMacro( ClosedLoop, int );
            vtkBooleanMacro(ClosedLoop, int);

            // Description:
            // A flag to indicate whether to show the Selected nodes
            // Default is to set it to false.
            virtual void SetShowSelectedNodes(int);vtkGetMacro( ShowSelectedNodes, int );
            vtkBooleanMacro(ShowSelectedNodes, int);

            //BTX
            // Description:
            // Get the points in this contour as a vtkPolyData.
            virtual vtkPolyData *GetContourRepresentationAsPolyData() = 0;
            //ETX

            // Description:
            // Get the nodes and not the intermediate points in this
            // contour as a vtkPolyData.
            void GetNodePolyData(vtkPolyData* poly);vtkSetMacro(RebuildLocator,bool);

            // get/plane orientation
            void SetOrientation(Plane orientation);
            Plane GetOrientation();

            /** \brief Set the shift of the representation in the orientation plane.
             * \param[in] value shift value in Nm
             *
             */
            void setShift(Nm value)
            { this->PlaneShift = 10 * value; }

            void setSlice(Nm value)
            { this->Slice = value; }

            // draw contour polygon, only used when we have a closed contour
            virtual void UseContourPolygon(bool value)=0;
          protected:
            vtkPlaneContourRepresentation();
            ~vtkPlaneContourRepresentation();

            // Selection tolerance for the handles
            int PixelTolerance;
            double WorldTolerance;

            vtkPointPlacer *PointPlacer;
            vtkContourLineInterpolator *LineInterpolator;

            int ActiveNode;

            int CurrentOperation;
            int ClosedLoop;

            // A flag to indicate whether to show the Selected nodes
            int ShowSelectedNodes;

            vtkPlaneContourRepresentationInternals *Internal;

            void AddNodeAtPositionInternal(double worldPos[3], double worldOrient[9], int displayPos[2]);
            void AddNodeAtPositionInternal(double worldPos[3], double worldOrient[9], double displayPos[2]);
            void SetNthNodeWorldPositionInternal(int n, double worldPos[3], double worldOrient[9]);

            // Description:
            // Given a world position and orientation, this computes the display position
            // using the renderer of this class.
            void GetRendererComputedDisplayPositionFromWorldPosition(double worldPos[3], double worldOrient[9],
                                                                     int displayPos[2]);
            void GetRendererComputedDisplayPositionFromWorldPosition(double worldPos[3], double worldOrient[9],
                                                                     double displayPos[2]);

            virtual void UpdateLines(int index);
            void UpdateLine(int idx1, int idx2);

            virtual int FindClosestPointOnContour(int X, int Y, double worldPos[3], int *idx);

            virtual void BuildLines()=0;

            // This method is called when something changes in the point
            // placer. It will cause all points to
            // be updates, and all lines to be regenerated.
            // Should be extended to detect changes in the line interpolator
            // too.
            virtual int UpdateContour();
            vtkTimeStamp ContourBuildTime;

            void ComputeMidpoint(double p1[3], double p2[3], double mid[3])
            {
              mid[0] = (p1[0] + p2[0]) / 2;
              mid[1] = (p1[1] + p2[1]) / 2;
              mid[2] = (p1[2] + p2[2]) / 2;
            }

            // Description:
            // Build a contour representation from externally supplied PolyData. This
            // is very useful when you use an external program to compute a set of
            // contour nodes, let's say based on image features. Subsequently, you want
            // to build and display a contour that runs through those points.
            // This method is protected and accessible only from
            // ContourWidget::Initialize( vtkPolyData * )
            virtual void Initialize(vtkPolyData *);

            //Description:
            // Adding a point locator to the representation to speed
            // up lookup of the active node when dealing with large datasets (100k+)
            vtkIncrementalOctreePointLocator *Locator;

            //Description:
            // Deletes the previous locator if it exists and creates
            // a new locator. Also deletes / recreates the attached data set.
            void ResetLocator();
            void BuildLocator();
            bool RebuildLocator;

            // iterates over the contour lines to check if line segments intersect, if so then deletes useless points
            // and closes the contour in the intersection point. checks for intersection with segment (n-2, n-1)
            bool CheckAndCutContourIntersection(void);

            // iterates over the contour lines to check if line segments intersect, if so then deletes useless points
            // and closes the contour in the intersection point. checks for intersection with segments (n-2, n-1) and (n-1,0)
            bool CheckAndCutContourIntersectionInFinalPoint(void);

            // same as previous method, but just checks (n-1, n) and (n, n+1). used only when translating node the node n
            // after the contour has been defined
            bool CheckContourIntersection(int);

            // check nodes if nodes a and b have the same coordinates
            bool CheckNodesForDuplicates(int, int);

            // implements shooting algorithm to know if a point is inside a closed polygon
            bool ShootingAlgorithm(int, int);

            // returns the shortest distance to the contour given a point in world coordinates
            double FindClosestDistanceToContour(int, int);

            // helper methods
            bool LineIntersection(int, double *, int*, bool *);
            void RemoveDuplicatedNodes();
            bool NodesIntersection(int, int);
            void TranslatePoints(double *);
            Plane Orientation;
            Nm PlaneShift;
            Nm Slice;
          private:
            vtkPlaneContourRepresentation(const vtkPlaneContourRepresentation&); //Not implemented
            void operator=(const vtkPlaneContourRepresentation&); //Not implemented
          };

        } // namespace Contour
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // _VTKPLANECONTOURREPRESENTATION_H_
