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

#include <Core/EspinaTypes.h>
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/EventHandler.h>
#include <GUI/Representations/PipelineSources.h>

// Qt
#include <QWidget>

class vtkRenderer;
class vtkProp;
class vtkRenderWindow;
class QVTKWidget;
class QPushButton;

namespace ESPINA
{
  class EspinaWidget;

  class EspinaGUI_EXPORT RenderView
  : public QWidget
  , public SelectableView
  {
    Q_OBJECT
  public:
    /** \brief RenderView class constructor.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     *
     */
    explicit RenderView(QWidget* parent = nullptr);

    /** \brief RenderView class virtual destructor.
     *
     */
    virtual ~RenderView();

    /** \brief Sets the view event handler.
     * \param[in] eventHandler, event handler smart pointer.
     *
     */
    void setEventHandler(EventHandlerSPtr eventHandler)
    { m_eventHandler = eventHandler; }

    /** \brief Returns the view's event handler.
     *
     */
    EventHandlerSPtr eventHandler() const
    { return m_eventHandler; }

    void setChannelSources(PipelineSources *channels);

    /** \brief Adds a representation manager to the view
     *
     */
    void addRepresentationManager(RepresentationManagerSPtr manager);

    /** \brief Removes a representation manager from the view
     *
     */
    void removeRepresentationManager(RepresentationManagerSPtr manager);

    /** \brief Resets the view to it's initial state.
     *  TODO: Rename to clear?
     */
    virtual void reset() = 0;

    /** \brief Adds a widget to the view.
     * \param[in] widget espina widget smart pointer.
     *
     */
    virtual void addWidget(EspinaWidgetSPtr widget);

    /** \brief Removes a widget to the view.
     * \param[in] widget espina widget smart pointer.
     *
     */
    virtual void removeWidget(EspinaWidgetSPtr widget);

    /** \brief Adds an actor to the vtkRenderer.
     * \param[in] actor vtkProp raw pointer.
     *
     */
    virtual void addActor(vtkProp *actor);

    /** \brief Removes an actor to the vtkRenderer.
     * \param[in] actor vtkProp raw pointer.
     *
     */
    virtual void removeActor(vtkProp *actor);

    /** \brief Returns the bounds in world coordinates that contains all of the objects in the view.
     * \param[in] cropToSceneBounds true to crop the bounds to the limits of the actual view, false otherwise.
     *
     */
    virtual Bounds previewBounds(bool cropToSceneBounds = true) const = 0;

    /** \brief Sets the view's cursor.
     * \param[in] cursor QCursor object.
     *
     */
    virtual void setCursor(const QCursor& cursor);

    /** \brief Returns the coordinates of the last mouse event.
     * \param[out] x coordinate.
     * \param[out] y coordinate.
     *
     */
    virtual void eventPosition(int &x, int &y);

    /** \brief Selects the NeuroItems specified in flags parameter whose voxels intersect the ones
     * in the mask given as parameter.
     * \param[in] flags NeuroItems selection flags.
     * \param[in] mask  Area selected to intersect with the items in the view.
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const Selector::SelectionMask &mask, bool multiselection = true) const;

    /** \brief Selects the NeuroIntems specified in flags parameter that has a voxel in the WORLD position
     * specified in the point parameter.
     * \param[in] flags NeuroItems selection flags.
     * \param[in] point Point in WORLD coordinates (not necessarily in the slice position of the view).
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const NmVector3 &point, bool multiselection = true) const;

    /** \brief Selects the NeuroItems specified in the flags parameter that has a voxel in the DISPLAY
     * position specified by the x and y parameters.
     * \param[in] flags, NeuroItems selection flags.
     * \param[in] x position in display coordinates.
     * \param[in] y position in display coordinates.
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const = 0;

    /** \brief Returns the raw pointer of the vtkRenderWindow of the view.
     *
     */
    virtual vtkRenderWindow *renderWindow() const;

    /** \brief Returns the raw pointer of the vtkRenderer of the view.
     *
     */
    virtual vtkRenderer *mainRenderer() const;

    /** \brief Resets the view's camera.
     *
     */
    virtual void resetCamera() = 0;

    /** \brief Returns the bounds of the scene.
     *
     */
    const Bounds sceneBounds() const
    {return m_sceneBounds;}

    /** \brief Returns the crosshair point.
     *
     */
    const NmVector3 crosshairPoint() const
    { return m_crosshairPoint; }

    /** \brief Returns the resolution (spacing) of the view.
     *
     */
    const NmVector3 sceneResolution() const
    {return m_sceneResolution;}

    /** \brief Centers the view on the given point.
     * \param[in] point to center the view.
     * \param[in] force if set to true, force render after setting the viewpoint.
     *
     */
    virtual void centerViewOn(const NmVector3& point, bool force=false) = 0;

    /** \brief Sets the contextual menu of the view.
     * \param[in] contextMenu ContextualMenu smart pointer.
     *
     */
    virtual void setContextualMenu(ContextualMenuSPtr contextMenu)
    { m_contextMenu = contextMenu; }

    /** \brief Struct used to store/restore camera state. Used in
     * "view state" snapshots.
     *
     */
    struct VisualState // RENAME
    {
      Plane     plane;
      int       slice;           // Only used in View2D
      NmVector3 cameraPosition;
      NmVector3 focalPoint;
      double    heightLength;      // Only used in View2D

      VisualState(): plane(Plane::UNDEFINED), slice(-1), cameraPosition(NmVector3{}), focalPoint(NmVector3{}), heightLength(0) {};
    };

    /** \brief Restores camera position and zoom.
     * \param[in] state VisualState struct with camera values.
     *
     */
    virtual void setVisualState(struct VisualState) = 0;

    /** \brief Returns the visual state of the view.
     *
     */
    virtual struct VisualState visualState() = 0;

    virtual void updateRepresentations(SegmentationAdapterList list);
    virtual void updateRepresentations(ChannelAdapterList list);
    virtual void updateRepresentations();

    /** \brief Helper method to create a QPushButton.
     * \param[in] icon of the button.
     * \param[in] tooltip of the button.
     *
     */
    static QPushButton *createButton(const QString& icon, const QString& tooltip);

  public slots:
    /** \brief Updates the view.
     *
     */
    virtual void updateView() = 0; // TODO: RENAME?

  signals:
    void sceneResolutionChanged();

  protected slots:
    /** \brief Resets the view's camera and updates the bounds of the scene.
     *
     */
    virtual void resetView();

    /** \brief Updates the representations of the given list of segmentations.
     * \param[in] selection list of segmentation adapter raw pointers.
     *
     */
    virtual void updateSelection(SegmentationAdapterList selection); // TODO: REVIEW

    /** \brief Updates the bounds of the scene after a channel has been added or deleted.
     *
     */
    virtual void updateSceneBounds();

  protected:
    /** \brief Updates the view when the selection changes.
     * \param[in] selection new selection.
     *
     */
    virtual void onSelectionSet(SelectionSPtr selection);

    /** \brief Generates and saves to disk an image of the actual view state.
     *
     */
    void takeSnapshot();

    /** \brief Resets the bounds of the scene.
     *
     */
    void resetSceneBounds();

    /** \brief Returns the number of active renderers for a given type of item.
     * \param[in] type RenderableType type.
     *
     */
    unsigned int numberActiveRepresentationManagers(Data::Type type);

  private slots:
    void onRenderRequest();

  private:
    virtual void configureManager(RepresentationManagerSPtr manager) {}

    void notifyResolutionChange();


  protected:
    EventHandlerSPtr m_eventHandler;

    QVTKWidget*  m_view;
    vtkSmartPointer<vtkRenderer> m_renderer;

    Bounds    m_sceneBounds;
    NmVector3 m_crosshairPoint;
    NmVector3 m_sceneResolution;// Min distance between 2 voxels in each axis

    ContextualMenuSPtr m_contextMenu;

    PipelineSources           *m_channelSources;
    RepresentationManagerSList m_managers;
    QList<EspinaWidgetSPtr>    m_widgets;

    bool m_sceneCameraInitialized;
  };

} // namespace ESPINA

#endif // ESPINA_RENDER_VIEW_H
