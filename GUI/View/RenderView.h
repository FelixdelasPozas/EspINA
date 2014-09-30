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

#ifndef ESPINARENDERVIEW_H
#define ESPINARENDERVIEW_H

// ESPINA
#include "GUI/View/SelectableView.h"
#include <Core/EspinaTypes.h>
#include <GUI/Representations/Renderers/RepresentationRenderer.h>
#include <GUI/Representations/Representation.h>
#include <GUI/Representations/Renderers/RepresentationRenderer.h>
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/EventHandler.h>

// Qt
#include <QWidget>
#include <QMenu>
#include <QFlags>

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
  protected:
    struct ChannelState
    {
      double     brightness;
      double     contrast;
      double     opacity;
      TimeStamp  timeStamp;
      QColor     stain;
      bool       visible;
      OutputSPtr output;

      RepresentationSList representations;
    };

    struct SegmentationState
    {
      Nm         depth;
      QColor     color;
      bool       highlited;
      TimeStamp  timeStamp;
      bool       visible;
      OutputSPtr output;

      RepresentationSList representations;
    };

  public:
    /** \brief RenderView class constructor.
     * \param[in] parent, raw pointer of the QWidget parent of this one.
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

    /** \brief Sets the view's color engine.
     * \param[in] engine, color engine smart pointer.
     *
     */
    void setColorEngine(ColorEngineSPtr engine)
    { m_colorEngine = engine; }

    /** \brief Returns the view's color engine.
     *
     */
    ColorEngineSPtr colorEngine() const
    { return m_colorEngine; }

    /** \brief Resets the view to it's initial state.
     *
     */
    virtual void reset() = 0;

    /** \brief Adds a channel to the view.
     * \param[in] channel, channel adapter raw pointer.
     *
     */
    virtual void add(ChannelAdapterPtr channel);

    /** \brief Adds a segmentation to the view.
     * \param[in] seg, segmentation adapter raw pointer.
     *
     */
    virtual void add(SegmentationAdapterPtr seg);

    /** \brief Removes a channel from the view.
     * \param[in] channel, channel adapter raw pointer.
     *
     */
    virtual void remove(ChannelAdapterPtr channel);

    /** \brief Removes a segmentation from the view.
     * \param[in] seg, segmentation adapter raw pointer.
     *
     */
    virtual void remove(SegmentationAdapterPtr seg);

    /** \brief Update the representations of the given channel.
     * \param[in] channel, channel adapter raw pointer.
     * \param[in] render, true to force a render after updating, false otherwise.
     *
     */
    virtual bool updateRepresentation(ChannelAdapterPtr channel, bool render = true);

    /** \brief Update the representations of the given segmentation.
     * \param[in] channel, segmentation adapter raw pointer.
     * \param[in] render, true to force a render after updating, false otherwise.
     *
     */
    virtual bool updateRepresentation(SegmentationAdapterPtr seg, bool render = true);

    /** \brief Implements SelectableView::updateRepresentations(ChannelAdapterList).
     *
     */
    virtual void updateRepresentations(ChannelAdapterList list);

    /** \brief Implements SelectableView::updateRepresentations(SegmentationAdapterList).
     *
     */
    virtual void updateRepresentations(SegmentationAdapterList list);

    /** \brief Implements SelectableView::updateRepresentations().
     *
     */
    virtual void updateRepresentations();

    /** \brief Adds a widget to the view.
     * \param[in] widget, espina widget smart pointer.
     *
     */
    virtual void addWidget(EspinaWidgetSPtr widget);

    /** \brief Removes a widget to the view.
     * \param[in] widget, espina widget smart pointer.
     *
     */
    virtual void removeWidget(EspinaWidgetSPtr widget);

    /** \brief Adds an actor to the vtkRenderer.
     * \param[in] actor, vtkProp raw pointer.
     *
     */
    virtual void addActor   (vtkProp *actor);

    /** \brief Removes an actor to the vtkRenderer.
     * \param[in] actor, vtkProp raw pointer.
     *
     */
    virtual void removeActor(vtkProp *actor);

    /** \brief Returns the bounds in world coordinates that contains all of the objects in the view.
     * \param[in] cropToSceneBounds, true to crop the bounds to the limits of the actual view, false otherwise.
     *
     */
    virtual Bounds previewBounds(bool cropToSceneBounds = true) const = 0;

    /** \brief Sets the view's cursor.
     * \param[in] cursor, QCursor object.
     *
     */
    virtual void setCursor(const QCursor& cursor);

    /** \brief Returns the coordinates of the last mouse event.
     * \param[out] x, x coordinate.
     * \param[out] y, y coordinate.
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
     * \param[in] x, x position in display coordinates.
     * \param[in] y, y position in display coordinates.
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
    virtual vtkRenderer     *mainRenderer() const;

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
     * \param[in] point, point to center the view.
     * \param[in] force, true to force a render after setting the viewpoint.
     *
     */
    virtual void centerViewOn(const NmVector3& point, bool force=false) = 0;

    /** \brief Sets the contextual menu of the view.
     * \param[in] contextMenu, ContextualMenu smart pointer.
     *
     */
    virtual void setContextualMenu(ContextualMenuSPtr contextMenu)
    { m_contextMenu = contextMenu; }

    /** \brief Adds the widgets of the renderer to the view's controls.
     * \param[in] renderer, renderer smart pointer.
     *
     */
    virtual void addRendererControls(RendererSPtr renderer) = 0;

    /** \brief Removes the widgets of the renderer from the view's controls.
     *
     */
    virtual void removeRendererControls(const QString name) = 0;

    /** \brief Creates and returns an instance of the given representation type for the given item.
     * \param[in] item, view item adapter raw pointer.
     * \param[in] type, type of representation to return.
     *
     */
    virtual RepresentationSPtr cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation) = 0;

    /** \brief Returns true if the segmentations are visible, false otherwise.
     *
     */
    bool segmentationsVisibility() const
    { return m_showSegmentations; }

    /** \brief Sets the segmentations visibility.
     * \param[in] visiblity, true to set visible, false otherwise.
     *
     */
    void setSegmentationsVisibility(bool visibility);

    /** \brief Activates the render with the given name.
     * \param[in] rendererName.
     *
     */
    virtual void activateRender(const QString &rendererName) = 0;

    /** \brief Dectivates the render with the given name.
     * \param[in] rendererName.
     *
     */
    virtual void deactivateRender(const QString &rendererName) = 0;

    /** \brief Sets the renderers for the view.
     * \param[in] renderers, list of renderer smart pointers.
     *
     */
    virtual void setRenderers(RendererSList renderers) = 0;

    /** \brief Returns the list of smart pointers of the renderers of the view.
     *
     */
    RendererSList renderers() const;

    /** \brief Sets the activation state of the renderers of the view.
     * \param[in] state, map of pair values of renderers' name and boolean state.
     *
     */
    void setRenderersState(QMap<QString, bool> state);

    /** \brief Struct used to store/restore camera state. Used in
     * "view state" snapshots.
     *
     */
    struct VisualState
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

  signals:
    void sceneResolutionChanged();

  public slots:
		/** \brief Updates the view.
		 *
		 */
    virtual void updateView() = 0;

  protected slots:
		/** \brief Updates the bounds of the scene after a channel has been added or deleted.
		 *
		 */
    virtual void updateSceneBounds();

		/** \brief Resets the view's camera and updates the bounds of the scene.
		 *
		 */
    virtual void resetView();

    /** \brief Updates the representations of the given list of segmentations.
     * \param[in] selection, list of segmentation adapter raw pointers.
     *
     */
    virtual void updateSelection(SegmentationAdapterList selection);

    /** \brief Helper method to create a QPushButton.
     * \param[in] icon, icon of the button.
     * \param[in] tooltip, tooltip of the button.
     *
     */
    QPushButton *createButton(const QString& icon, const QString& tooltip);

  protected:
    /** \brief Updates the view when the selection changes.
     * \param[in] selection, new selection.
     *
     */
    virtual void onSelectionSet(SelectionSPtr selection);

    /** \brief Overrides QWidget::showEvent().
     *
     */
    virtual void showEvent(QShowEvent *event) override;

    /** \brief Gernerates and saves to disk an image of the actual view state.
     *
     */
    void takeSnapshot();

    /** \brief Returns the suggested opacity for a channel.
     *
     */
    double suggestedChannelOpacity();

    /** \brief Updates the channel's opacity value.
     *
     */
    virtual void updateChannelsOpacity() = 0;

    /** \brief Resets the bounds of the scene.
     *
     */
    void resetSceneBounds();

    /** \brief Removes the channels representations from the view.
     * \param[in] state, map of the items and their representations in the view.
     *
     */
    void removeRepresentations(ChannelState &state);

    /** \brief Removes the segmentations representations from the view.
     * \param[in] state, map of the items and their representations in the view.
     *
     */
    void removeRepresentations(SegmentationState &state);

    /** \brief Returns the number of active renderers for a given type of item.
     * \param[in] type, RenderableType type.
     *
     */
    unsigned int numEnabledRenderersForViewItem(RenderableType type);

  protected:
    EventHandlerSPtr m_eventHandler;
    ColorEngineSPtr  m_colorEngine;

    QVTKWidget*  m_view;
    vtkSmartPointer<vtkRenderer> m_renderer;

    Bounds    m_sceneBounds;
    NmVector3 m_crosshairPoint;
    NmVector3 m_sceneResolution;// Min distance between 2 voxels in each axis

    ContextualMenuSPtr m_contextMenu;

    QMap<ChannelAdapterPtr,      ChannelState>      m_channelStates;
    QMap<SegmentationAdapterPtr, SegmentationState> m_segmentationStates;

    RendererSList m_renderers;
    QList<EspinaWidgetSPtr> m_widgets;

    bool m_sceneCameraInitialized;
    bool m_showSegmentations;
  };

} // namespace ESPINA

#endif // ESPINARENDERVIEW_H
