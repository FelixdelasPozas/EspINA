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

#ifndef ESPINA_RENDER_VIEW_H
#define ESPINA_RENDER_VIEW_H

// ESPINA
#include "GUI/View/SelectableView.h"

#include <Core/Types.h>
#include <GUI/Types.h>

#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/View/EventHandler.h>
#include "ViewState.h"
#include <GUI/Representations/ModelSources.h>

// Qt
#include <QWidget>
#include <QElapsedTimer>

class vtkRenderer;
class vtkProp;
class vtkRenderWindow;
class QVTKWidget;
class QPushButton;

namespace ESPINA
{
  class EspinaGUI_EXPORT RenderView
  : public QWidget
  , public SelectableView
  {
    Q_OBJECT
  public:
    struct CameraState
    {
      Plane     plane;
      int       slice;           // Only used in View2D
      NmVector3 cameraPosition;
      NmVector3 focalPoint;
      NmVector3 upVector;
      double    heightLength;    // Only used in View2D

      CameraState(): plane(Plane::UNDEFINED), slice(-1), cameraPosition(NmVector3{}), focalPoint(NmVector3{}), upVector(NmVector3{}), heightLength(0) {};
    };

  public:
    /** \brief RenderView class virtual destructor.
     *
     */
    virtual ~RenderView();

    /** \brief View type (2D or 3D)
     *
     */
    ViewType type() const
    { return m_type; }

    /** \brief Adds a representation manager to the view
     *
     */
    void addRepresentationManager(GUI::Representations::RepresentationManagerSPtr manager);

    /** \brief Removes a representation manager from the view
     *
     */
    void removeRepresentationManager(GUI::Representations::RepresentationManagerSPtr manager);

    /** \brief Returns the bounds in world coordinates that contains all of the objects in the view.
     * \param[in] cropToSceneBounds true to crop the bounds to the limits of the actual view, false otherwise.
     *
     */
    virtual Bounds previewBounds(bool cropToSceneBounds = true) const = 0;

    /** \brief Returns the coordinates of the last mouse event.
     * \param[out] x coordinate.
     * \param[out] y coordinate.
     *
     */
    void eventPosition(int &x, int &y);

    /** \brief Returns the world coordinates of the last mouse event.
     *
     */
    NmVector3 worldEventPosition();

    /** \brief Returns the world coordinates of the last mouse event.
     *
     */
    NmVector3 worldEventPosition(const QPoint &pos);

    /** \brief Selects the view items whose types are defined by flags and have a valid representation at selection point
     * \param[in] flags view item types to be selected.
     * \param[in] point position in WORLD coordinates.
     * \param[in] multiselection if true several view items may be returned.
     *
     */
    virtual Selector::Selection pick(const Selector::SelectionFlags flags, const NmVector3 &point, bool multiselection = true) const;

    /** \brief Selects the view items whose types are defined by flags and have a valid representation at selection point
     * \param[in] flags view item types to be selected.
     * \param[in] x position in DISPLAY coordinates.
     * \param[in] y position in DISPLAY coordinates.
     * \param[in] multiselection if true several view items may be returned.
     *
     */
    virtual Selector::Selection pick(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const;

    /** \brief Returns the raw pointer of the vtkRenderWindow of the view.
     *
     */
    virtual vtkRenderWindow *renderWindow() const;

    /** \brief Returns the raw pointer of the vtkRenderer of the view.
     *
     */
    virtual vtkRenderer *mainRenderer() const = 0;

    /** \brief Returns the bounds of the scene.
     *
     */
    const Bounds sceneBounds() const;

    /** \brief Returns the crosshair point.
     *
     */
    const NmVector3 crosshair() const;

    /** \brief Returns the resolution (spacing) of the view.
     *
     */
    const NmVector3 sceneResolution() const;

    /** \brief Sets the contextual menu of the view.
     * \param[in] contextMenu to be displayed on right button click
     *
     */
    void setContextualMenu(ContextualMenuSPtr contextMenu)
    { m_contextMenu = contextMenu; }

    /** \brief Restores camera position and zoom.
     * \param[in] state VisualState struct with camera values.
     *
     */
    virtual void setCameraState(CameraState camera) = 0;

    /** \brief Returns the visual state of the view.
     *
     */
    virtual CameraState cameraState() = 0;

    /** \brief Adds an actor to the vtkRenderer.
     * \param[in] actor vtkProp raw pointer.
     *
     */
    virtual void addActor(vtkProp *actor) = 0;

    /** \brief Removes an actor to the vtkRenderer.
     * \param[in] actor vtkProp raw pointer.
     *
     */
    virtual void removeActor(vtkProp *actor) = 0;

    /** \brief Helper method to create a render view action buttons
     * \param[in] icon of the button.
     * \param[in] tooltip of the button.
     *
     */
    static QPushButton *createButton(const QString& icon, const QString& tooltip);

    /** \brief Returns the name of the view, for debugging purposes.
     *
     */
    virtual const QString viewName() const = 0;

    /** \brief Returns the ViewState used by the view.
     *
     */
    GUI::View::ViewState &state() const;

  public slots:
    /** \brief Resets the camera using the camera reset button of the view.
     *
     */
    virtual void resetCamera();

    /** \brief Request a graphical refresh of the current view content
     *
     */
    void refresh();

  signals:
    void crosshairChanged(NmVector3);

    void crosshairPlaneChanged(Plane, Nm);

    void viewFocusedOn(NmVector3 focusPoint);

  protected slots:

    /** \brief Resets the view to it's initial state.
     *
     */
    virtual void reset();

    /** \brief Tries to update the view when a manager request for a render.
     *
     */
    void onRenderRequest();

  protected:
    /** \brief RenderView class constructor.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     * \param[in] type type of view 2D or 3D
     * \param[in] parent QWidget parent of this one.
     *
     */
    explicit RenderView(GUI::View::ViewState &state, ViewType type, QWidget *parent = nullptr);

    /** \brief Returns the point world coordinates corresponding to the given ones in the given renderer.
     * \param[in] renderer vtkRenderer used for transformation.
     * \param[in] x x axis display coordinates.
     * \param[in] y y axis display coordinates.
     * \param[in] z z axis display coordinates.
     *
     */
    NmVector3 toWorldCoordinates(vtkRenderer *renderer, int x, int y, int z) const;

    /** \brief Updates the view when the selection changes.
     * \param[in] selection new selection.
     *
     */
    virtual void onSelectionSet(SelectionSPtr selection);

    /** \brief Updates the selection of items.
     * \param[in] append if true the elements picked will be merged with the ones currently
     *  selected, if false the elements picked will be the new selection.
     *
     *  If an item is selected and also is on the picked list the merge will deselect the item.
     *
     */
    void selectPickedItems(int x, int y, bool append);

    /** \brief Generates and saves to disk an image of the actual view state.
     *
     */
    void takeSnapshot();

    /** \brief Shows tool tip for segmentations at position (x, y)
     * \param[in] x display coordinate
     * \param[in] y display coordinate
     *
     */
    void showSegmentationTooltip(const int x, const int y);

    bool hasVisibleRepresentations() const;

    bool hasChannelSources() const;

    virtual void resetImplementation() = 0;

    EventHandlerSPtr eventHandler() const;

    bool eventHandlerFilterEvent(QEvent *event);

  private:
    /** \brief Renders the last frame in all the managers.
     * \param[in] frame frame to render
     * \param[in] managers managers to render the frame.
     *
     */
    void renderFrame(GUI::Representations::FrameCSPtr frame, GUI::Representations::RepresentationManagerSList managers);

    virtual Selector::Selection pickImplementation(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const = 0;

    virtual void configureManager(GUI::Representations::RepresentationManagerSPtr manager) {}

    virtual void normalizeWorldPosition(NmVector3 &point) const {}

    virtual void updateViewActions(GUI::Representations::RepresentationManager::ManagerFlags flags) = 0;

    virtual void resetCameraImplementation() = 0;

    virtual void refreshViewImplementation() {}

    virtual bool isCrosshairPointVisible() const = 0;

    void connectSignals();

    GUI::Representations::RepresentationManagerSList pendingManagers() const;

    unsigned int activeManagers() const;

    GUI::Representations::RepresentationManagerSList pendingManagers(GUI::Representations::RepresentationManagerSList managers) const;

    GUI::Representations::FrameCSPtr latestReadyFrame(GUI::Representations::RepresentationManagerSList managers) const;

    void display(GUI::Representations::RepresentationManagerSList managers, const TimeStamp time);

    GUI::Representations::RepresentationManager::ManagerFlags managerFlags() const;

    void deleteInactiveWidgetManagers();

    virtual void moveCamera(const NmVector3 &point) = 0;

  private slots:
    virtual void onCrosshairChanged(const GUI::Representations::FrameCSPtr frame) = 0;

    virtual void onSceneResolutionChanged(const NmVector3 &resolution) = 0;

    virtual void onSceneBoundsChanged(const Bounds &bounds) = 0;

    virtual void addSliceSelectors(SliceSelectorSPtr  widget, SliceSelectionType selector) {};

    virtual void removeSliceSelectors(SliceSelectorSPtr widget) {};

    void onWidgetsAdded(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

    void onWidgetsRemoved(GUI::Representations::Managers::TemporalPrototypesSPtr factory, const GUI::Representations::FrameCSPtr frame);

    void delayedWidgetsShow();

  protected:
    using TemporalPrototypesSPtr     = GUI::Representations::Managers::TemporalPrototypesSPtr;
    using RepresentationManagerSPtr  = GUI::Representations::RepresentationManagerSPtr;
    using RepresentationManagerSList = GUI::Representations::RepresentationManagerSList;

    ContextualMenuSPtr         m_contextMenu;
    QVTKWidget                *m_view;
    RepresentationManagerSList m_managers;
    unsigned int m_lastFrameActiveManagers;

  private:
    /** \brief vtkImageData to QImage conversion.
     * \param[in] image vtkImageData object pointer.
     *
     */
    QImage vtkImageDataToQImage(vtkImageData* image) const;

    GUI::View::ViewState &m_state;
    SelectionSPtr         m_selection;

    QElapsedTimer m_timer;
    ViewType      m_type;
    GUI::Representations::FrameCSPtr m_latestFrame;
    QMap<TemporalPrototypesSPtr, RepresentationManagerSPtr> m_temporalManagers;
  };

} // namespace ESPINA

#endif // ESPINA_RENDER_VIEW_H
