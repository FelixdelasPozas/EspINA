#ifndef VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H
#define VTKRECTANGULARBOUNDINGBOXREPRESENTATION_H

#include <Core/Utils/Vector3.hxx>
#include "CountingFramePlugin_Export.h"

#include "vtkWidgetRepresentation.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>


class vtkLookupTable;
class vtkPolyDataAlgorithm;
class vtkActor;
class vtkPolyDataMapper;
class vtkLineSource;
class vtkCellPicker;
class vtkProperty;
class vtkPolyData;
class vtkPoints;
class vtkPolyDataAlgorithm;
class vtkPointHandleRepresentation3D;
class vtkTransform;
class vtkPlanes;
class vtkBox;
class vtkDoubleArray;
class vtkMatrix4x4;

class CountingFramePlugin_EXPORT vtkCountingFrameSliceRepresentation
: public vtkWidgetRepresentation
{
  protected:
    //BTX
    enum EDGE {LEFT, TOP, RIGHT, BOTTOM};
    //ETX

  public:
    // Description:
    // Standard methods for the class.
    vtkTypeMacro(vtkCountingFrameSliceRepresentation,vtkWidgetRepresentation);
    void PrintSelf(ostream& os, vtkIndent indent);

    /** \brief Resets the margins and recreates the representation.
     *
     */
    void reset();

    // Description:
    // Get the outline properties (the outline of the box). The
    // properties of the outline when selected and normal can be
    // set.
    vtkGetObjectMacro(SelectedInclusionProperty,vtkProperty);

    /** \brief Sets the slice of the representation.
     * \param[in] pos slice position in Nm.
     *
     */
    virtual void SetSlice(ESPINA::Nm pos) = 0;

    /** \brief Defines the counting frame representation properties.
     * \param[in] region CF geometry data.
     * \param[in] inclusionOffset offset of the inclusion margins.
     * \param[in] exclusionOffset offset of the exclusion margins.
     * \param[in] slicingStep spacing between slices.
     *
     */
    virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> region,
                                  ESPINA::Nm inclusionOffset[3],
                                  ESPINA::Nm exclusionOffset[3],
                                  ESPINA::NmVector3 slicingStep);

    /** \brief Highlights/dims the representation.
     * \param[in] highlight true to highlight and false to dim the representation.
     *
     */
    virtual void SetHighlighted(bool highlight);

    /** \brief Sets the distance from the slice point where the representation will be shown.
     * \param[in] depth distance in Nm.
     *
     */
    void SetRepresentationDepth(ESPINA::Nm depth);

    // Description:
    // These are methods to communicate with the 3d_widget
    vtkSetVector3Macro(InclusionOffset, double);
    vtkGetVector3Macro(InclusionOffset, double);
    vtkSetVector3Macro(ExclusionOffset, ESPINA::Nm);
    vtkGetVector3Macro(ExclusionOffset, ESPINA::Nm);

    // Description:
    // These are methods that satisfy vtkWidgetRepresentation's API.
    virtual void PlaceWidget(double bounds[6]);
    virtual void BuildRepresentation();
    virtual int  ComputeInteractionState(int X, int Y, int modify=0);
    virtual void StartWidgetInteraction(double e[2]);
    virtual void WidgetInteraction(double e[2]);
    virtual double *GetBounds();

    // Description:
    // Methods supporting, and required by, the rendering process.
    virtual void ReleaseGraphicsResources(vtkWindow*);
    virtual int  RenderOpaqueGeometry(vtkViewport*);
    virtual int  RenderTranslucentPolygonalGeometry(vtkViewport*);
    virtual int  HasTranslucentPolygonalGeometry();

    //BTX - used to manage the state of the widget
    enum {Outside=0,
      MoveLeft, MoveRight, MoveTop, MoveBottom, Translating
    };
    //ETX

    // Description:
    // The interaction state may be set from a widget (e.g., vtkBoxWidget2) or
    // other object. This controls how the interaction with the widget
    // proceeds. Normally this method is used as part of a handshaking
    // process with the widget: First ComputeInteractionState() is invoked that
    // returns a state based on geometric considerations (i.e., cursor near a
    // widget feature), then based on events, the widget may modify this
    // further.
    void SetInteractionState(int state);

  protected:
    /** \brief vtkCountingFrameSliceRepresentation class constructor.
     *
     */
    vtkCountingFrameSliceRepresentation();

    /** \brief vtkCountingFrameSliceRepresentation class destructor.
     *
     */
    ~vtkCountingFrameSliceRepresentation();

    // Manage how the representation appears
    double LastEventPosition[3];                           /** last event position coordinates. */

    // Counting Region Edge
    vtkSmartPointer<vtkActor>          EdgeActor[4];       /** actors of the edges lines. */
    vtkSmartPointer<vtkPolyDataMapper> EdgeMapper[4];      /** mappers for the edges actors. */
    vtkSmartPointer<vtkPolyData> 	     EdgePolyData[4];    /** data for the edges actors. */
    vtkSmartPointer<vtkPoints>	       Vertex;             /** data of the current widget placement. */

    /** \brief Highlights/dims the given edge actor.
     * \param[in] actor actor to modify its properties.
     *
     */
    void HighlightEdge(vtkSmartPointer<vtkActor> actor);

    // Do the picking
    vtkSmartPointer<vtkCellPicker> EdgePicker;              /** edge actors picker. */
    vtkSmartPointer<vtkCellPicker> LastPicker;              /** last picker used. */
    vtkSmartPointer<vtkActor>      CurrentEdge;             /** current picked edge. */

    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkProperty *InclusionEdgeProperty;                     /** properties of the inclusion edges actors. */
    vtkProperty *ExclusionEdgeProperty;                     /** properties of the exclusion edges actors. */
    vtkProperty *SelectedInclusionProperty;                 /** properties of the highlighted inclusion edges actors. */
    vtkProperty *SelectedExclusionProperty;                 /** properties of the highlighted exclusion edges actors. */
    vtkProperty *InvisibleProperty;                         /** properties for when the edges are invisible. */

    /** \brief Creates the default properties for the edges actors.
     *
     */
    virtual void CreateDefaultProperties();

    /** \brief Returns the bounds of the specified slice.
     * \param[in] regionSlice slice number.
     * \param[out] bounds given slice bounds.
     *
     */
    void regionBounds(int regionSlice, ESPINA::Nm bounds[6]) const;

    /** \brief Returns the left coordinate in Nm of the given slice without the correction.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm realLeftEdge  (int slice=0) = 0;

    /** \brief Returns the top coordinate in Nm of the given slice without the correction.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm realTopEdge   (int slice=0) = 0;

    /** \brief Returns the right coordinate in Nm of the given slice without the correction.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm realRightEdge (int slice=0) = 0;

    /** \brief Returns the bottom coordinate in Nm of the given slice without the correction.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm realBottomEdge(int slice=0) = 0;

    /** \brief Returns the corrected left coordinate in Nn given the slice number.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm leftEdge  (int slice=0) = 0;

    /** \brief Returns the corrected top coordinate in Nn given the slice number.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm topEdge   (int slice=0) = 0;

    /** \brief Returns the corrected right coordinate in Nn given the slice number.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm rightEdge (int slice=0) = 0;

    /** \brief Returns the corrected bottom coordinate in Nn given the slice number.
     * \param[in] slice slice number.
     *
     */
    virtual ESPINA::Nm bottomEdge(int slice=0) = 0;

    /** \brief Returns the slice number in the Z direction for the given Z coordinate in Nm.
     * \param[in] pos Z coordinate in Nm.
     *
     */
    const int sliceNumber(ESPINA::Nm pos) const;

    /** \brief Creates the face representation.
     *
     */
    virtual void CreateRegion() = 0;

    /** \brief Helper method for the translation of the left edge.
     * \param[in] p1 previous position.
     * \param[in] p2 final position.
     *
     */
    virtual void MoveLeftEdge  (double *p1, double *p2) = 0;

    /** \brief Helper method for the translation of the right edge.
     * \param[in] p1 previous position.
     * \param[in] p2 final position.
     *
     */
    virtual void MoveRightEdge (double *p1, double *p2) = 0;

    /** \brief Helper method for the translation of the top edge.
     * \param[in] p1 previous position.
     * \param[in] p2 final position.
     *
     */
    virtual void MoveTopEdge   (double *p1, double *p2) = 0;

    /** \brief Helper method for the translation of the bottom edge.
     * \param[in] p1 previous position.
     * \param[in] p2 final position.
     *
     */
    virtual void MoveBottomEdge(double *p1, double *p2) = 0;

  protected:
    vtkSmartPointer<vtkPolyData> Region; /** representation face. */
    ESPINA::Nm Slice;                    /** slice coordinate in Nm. */
    ESPINA::Nm Depth;                    /** distance from the slice coordinate to show the plane. */
    ESPINA::NmVector3 SlicingStep;       /** distance between slices. */

    ESPINA::Nm InclusionOffset[3];       /** exclusion margins offset. */
    ESPINA::Nm ExclusionOffset[3];       /** inclusion margins offset. */

    int NumPoints; /** number of points. */
    int NumSlices; /** number of slices */

  private:
    vtkCountingFrameSliceRepresentation(const vtkCountingFrameSliceRepresentation&) = delete;
    void operator=(const vtkCountingFrameSliceRepresentation&) = delete;
};

#endif
