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

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/QtWidget/IEspinaView.h"
#include "GUI/QtWidget/SegmentationContextualMenu.h"
#include <GUI/Renderers/Renderer.h>
#include "Core/EspinaTypes.h"
#include "GUI/Pickers/ISelector.h"
#include <GUI/Representations/GraphicalRepresentation.h>

// Qt
#include <QWidget>
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

  class EspinaGUI_EXPORT EspinaRenderView
  : public QWidget
  , public IEspinaView
  {
    Q_OBJECT
  protected:
    struct ChannelState
    {
      double     brightness;
      double     contrast;
      double     opacity;
      OutputSPtr output;
      QColor     stain;
      bool       visible;

      ChannelGraphicalRepresentationList representations;
    };

    struct SegmentationState
    {
      Nm         depth;
      QColor     color;
      bool       highlited;
      OutputSPtr output;
      bool       visible;

      SegmentationGraphicalRepresentationList representations;
    };

  public:
    explicit EspinaRenderView(ViewManager *vm, QWidget* parent = 0);
    virtual ~EspinaRenderView();

    virtual void reset() = 0;

    virtual void addChannel   (ChannelPtr channel);
    virtual void removeChannel(ChannelPtr channel);
    virtual bool updateChannelRepresentation (ChannelPtr  channel, bool render = true);
    virtual void updateChannelRepresentations(ChannelList list =  ChannelList());

    virtual void addSegmentation   (SegmentationPtr seg);
    virtual void removeSegmentation(SegmentationPtr seg);
    virtual bool updateSegmentationRepresentation (SegmentationPtr  seg, bool render = true);
    virtual void updateSegmentationRepresentations(SegmentationList list = SegmentationList());

    virtual void addWidget   (EspinaWidget *widget) = 0;
    virtual void removeWidget(EspinaWidget *widget) = 0;

    virtual void addActor   (vtkProp *actor);
    virtual void removeActor(vtkProp *actor);

    virtual void previewBounds(Nm bounds[6], bool cropToSceneBounds = true);

    virtual void setCursor(const QCursor& cursor);

    virtual void eventPosition(int &x, int &y);

    virtual ISelector::PickList pick(ISelector::PickableItems filter, ISelector::DisplayRegionList regions) = 0;

    virtual void worldCoordinates(const QPoint &displayPos, double worldPos[3]) = 0;

    virtual void setSelectionEnabled(bool enable) = 0;

    virtual vtkRenderWindow *renderWindow();
    virtual vtkRenderer     *mainRenderer();

    virtual void updateView() = 0;
    virtual void resetCamera() = 0;

    const Nm *sceneBounds() const {return m_sceneBounds;}
    const Nm *sceneResolution() const {return m_sceneResolution;}

    virtual void centerViewOn(Nm center[3], bool) = 0;
    virtual void showCrosshairs(bool visible) = 0;

    virtual void setViewType(PlaneType plane);
    virtual PlaneType getViewType();

    virtual void setContextualMenu(SegmentationContextualMenuSPtr contextMenu)
    { m_contextMenu = contextMenu; }

    virtual void addRendererControls(IRendererSPtr renderer) = 0;
    virtual void removeRendererControls(const QString name) = 0;

    virtual GraphicalRepresentationSPtr cloneRepresentation(GraphicalRepresentationSPtr prototype) = 0;

    bool segmentationsVisibility() const
    { return m_showSegmentations; }

    void setSegmentationsVisibility(bool visibility);

  protected slots:
    virtual void updateSceneBounds();
    virtual void resetView();
    virtual void updateSelection(ViewManager::Selection selection, bool render);

  protected:
    virtual void showEvent(QShowEvent *event);

    void takeSnapshot(vtkSmartPointer<vtkRenderer> renderer);

    double suggestedChannelOpacity();
    virtual void updateChannelsOpactity() = 0;

    void resetSceneBounds();

    void removeGraphicalRepresentations(ChannelState      &state);
    void removeGraphicalRepresentations(SegmentationState &state);

  protected:
    ViewManager *m_viewManager;
    QVTKWidget  *m_view;
    vtkSmartPointer<vtkRenderer> m_renderer;

    Nm m_sceneBounds[6];
    Nm m_sceneResolution[3];// Min distance between 2 voxels in each axis
    PlaneType m_plane;

    unsigned int m_numEnabledSegmentationRenders;
    unsigned int m_numEnabledChannelRenders;

    SegmentationContextualMenuSPtr m_contextMenu;

    QMap<ChannelPtr,      ChannelState>      m_channelStates;
    QMap<SegmentationPtr, SegmentationState> m_segmentationStates;

    QMap<QPushButton *, IRendererSPtr> m_renderers;

    bool m_sceneCameraInitialized;
    bool m_showSegmentations;
  };

} // namespace EspINA

#endif // ESPINARENDERVIEW_H
