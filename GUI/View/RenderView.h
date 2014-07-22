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

#include <QWidget>
#include "GUI/View/SelectableView.h"

// ESPINA
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
    explicit RenderView(QWidget* parent = 0);
    virtual ~RenderView();

    void setEventHandler(EventHandlerSPtr eventHandler)
    { m_eventHandler = eventHandler; }

    EventHandlerSPtr eventHandler() const
    { return m_eventHandler; }

    void setColorEngine(ColorEngineSPtr engine)
    { m_colorEngine = engine; }

    ColorEngineSPtr colorEngine() const
    { return m_colorEngine; }

    virtual void reset() = 0;

    virtual void add(ChannelAdapterPtr channel);
    virtual void add(SegmentationAdapterPtr seg);

    virtual void remove(ChannelAdapterPtr channel);
    virtual void remove(SegmentationAdapterPtr seg);

    virtual bool updateRepresentation (ChannelAdapterPtr  channel, bool render = true);
    virtual bool updateRepresentation (SegmentationAdapterPtr  seg, bool render = true);
    virtual void updateRepresentations(ChannelAdapterList list);
    virtual void updateRepresentations(SegmentationAdapterList list);

    virtual void updateRepresentations();

    virtual void addWidget   (EspinaWidgetSPtr widget);
    virtual void removeWidget(EspinaWidgetSPtr widget);

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const = 0;

    virtual void setCursor(const QCursor& cursor);

    virtual void eventPosition(int &x, int &y);

    /* \brief Selects the NeuroItems specified in flags parameter whose voxels intersect the ones
     * in the mask given as parameter.
     * \param[in] flags NeuroItems selection flags.
     * \param[in] mask  Area selected to intersect with the items in the view.
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const Selector::SelectionMask &mask, bool multiselection = true) const;

    /* \brief Selects the NeuroIntems specified in flags parameter that has a voxel in the WORLD position
     * specified in the point parameter.
     * \param[in] flags NeuroItems selection flags.
     * \param[in] point Point in WORLD coordinates (not necessarily in the slice position of the view).
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const NmVector3 &point, bool multiselection = true) const;

    /* \brief Selects the NeuroItems specified in the flags parameter that has a voxel in the DISPLAY
     * position specified by the x and y parameters.
     * \param[in] flags NeuroItems selection flags.
     * \param[in] x     x position in display coordinates.
     * \param[in] y     y position in display coordinates.
     *
     */
    virtual Selector::Selection select(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection = true) const = 0;

    virtual vtkRenderWindow *renderWindow() const;
    virtual vtkRenderer     *mainRenderer() const;

    virtual void resetCamera() = 0;

    const Bounds sceneBounds() const
    {return m_sceneBounds;}

    const NmVector3 crosshairPoint() const
    { return m_crosshairPoint; }

    const NmVector3 sceneResolution() const
    {return m_sceneResolution;}

    virtual void centerViewOn(const NmVector3& point, bool force=false) = 0;

    virtual void setContextualMenu(ContextualMenuSPtr contextMenu)
    { m_contextMenu = contextMenu; }

    virtual void addRendererControls(RendererSPtr renderer) = 0;

    virtual void removeRendererControls(const QString name) = 0;

    virtual RepresentationSPtr cloneRepresentation(ViewItemAdapterPtr item, Representation::Type representation) = 0;

    bool segmentationsVisibility() const
    { return m_showSegmentations; }

    void setSegmentationsVisibility(bool visibility);

    virtual void activateRender(const QString &rendererName) = 0;
    virtual void deactivateRender(const QString &rendererName) = 0;

    /* \brief Struct used to store/restore camera state. Used in
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

    /* \brief Restores camera position and zoom.
     * \param[in] state VisualState struct with camera values.
     *
     */
    virtual void setVisualState(struct VisualState) = 0;
    virtual struct VisualState visualState() = 0;

  signals:
    void sceneResolutionChanged();

  public slots:
    virtual void updateView() = 0;

  protected slots:
    virtual void updateSceneBounds();

    virtual void resetView();

    virtual void updateSelection(SegmentationAdapterList selection);

    QPushButton *createButton(const QString& icon, const QString& tooltip);

  protected:
    virtual void onSelectionSet(SelectionSPtr selection); 

    virtual void showEvent(QShowEvent *event);

    void takeSnapshot(vtkSmartPointer<vtkRenderer> renderer);

    double suggestedChannelOpacity();

    virtual void updateChannelsOpactity() = 0;

    void resetSceneBounds();

    void removeRepresentations(ChannelState      &state);
    void removeRepresentations(SegmentationState &state);

    unsigned int numEnabledRenderersForViewItem(RenderableType);

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
