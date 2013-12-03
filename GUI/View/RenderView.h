/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/Representations/Representation.h>
#include <GUI/Representations/Renderers/Renderer.h>
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/ColorEngines/ColorEngine.h>

// Qt
#include <QMenu>
#include <QFlags>

class vtkRenderer;
class vtkProp;
class vtkRenderWindow;
class QVTKWidget;
class QPushButton;

namespace EspINA
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

    void setSelector(SelectorSPtr selector)
    { m_selector = selector; }

    SelectorSPtr selector() const
    { return m_selector; }

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

    virtual void addWidget   (EspinaWidget *widget) = 0;
    virtual void removeWidget(EspinaWidget *widget) = 0;

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual Bounds previewBounds(bool cropToSceneBounds = true) const = 0;

    virtual void setCursor(const QCursor& cursor);

    virtual void eventPosition(int &x, int &y);

    // DEPRECATED
    virtual Selector::SelectionList pick(Selector::SelectionFlags filter, Selector::DisplayRegionList regions) = 0;

    virtual Selector::Selection select(Selector::SelectionFlags flags, Selector::SelectionMask mask) = 0;

//     virtual Selection currentSelection() const;
    //virtual void worldCoordinates(const QPoint &displayPos, double worldPos[3]) = 0;

    virtual vtkRenderWindow *renderWindow();
    virtual vtkRenderer     *mainRenderer();

    virtual void updateView() = 0;
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

  protected slots:
    virtual void updateSceneBounds();

    virtual void resetView();

    virtual void updateSelection(SegmentationAdapterList selection);

  protected:
    virtual void onSelectionSet(SelectionSPtr selection); 

    virtual void showEvent(QShowEvent *event);

    void takeSnapshot(vtkSmartPointer<vtkRenderer> renderer);

    double suggestedChannelOpacity();

    virtual void updateChannelsOpactity() = 0;

    void resetSceneBounds();

    void removeRepresentations(ChannelState      &state);
    void removeRepresentations(SegmentationState &state);

  protected:
    SelectorSPtr    m_selector;
    ColorEngineSPtr m_colorEngine;

    QVTKWidget*  m_view;
    vtkSmartPointer<vtkRenderer> m_renderer;

    Bounds    m_sceneBounds;
    NmVector3 m_crosshairPoint;
    NmVector3 m_sceneResolution;// Min distance between 2 voxels in each axis

    unsigned int m_numEnabledChannelRenders;
    unsigned int m_numEnabledSegmentationRenders;

    ContextualMenuSPtr m_contextMenu;

    QMap<ChannelAdapterPtr,      ChannelState>      m_channelStates;
    QMap<SegmentationAdapterPtr, SegmentationState> m_segmentationStates;

    QMap<QPushButton *, RendererSPtr> m_renderers;

    bool m_sceneCameraInitialized;
    bool m_showSegmentations;
  };

} // namespace EspINA

#endif // ESPINARENDERVIEW_H
