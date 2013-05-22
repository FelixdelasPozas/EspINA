/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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

//----------------------------------------------------------------------------
// File:    VolumeView.h
// Purpose: Display 3D representations for model's elements
//----------------------------------------------------------------------------

#ifndef VOLUMEVIEW_H
#define VOLUMEVIEW_H

#include "Core/EspinaTypes.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include "GUI/Renderers/Renderer.h"
#include "GUI/Representations/GraphicalRepresentation.h"
#include "GUI/ViewManager.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

#include <QPushButton>

class vtkAbstractWidget;
class QVTKWidget;

//Forward declaration
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QScrollBar;
class vtkPropPicker;

namespace EspINA
{
  class ColorEngine;
  class IViewWidget;

  class VolumeView
  : public EspinaRenderView
  {
    Q_OBJECT
  public:
    class Settings
    {
      const QString RENDERERS;
    public:
      explicit Settings(const EspinaFactory *factory,
                        const QString        prefix=QString(),
                        VolumeView          *parent=NULL);

      void setRenderers(IRendererList values);
      IRendererList renderers() const;

    private:
      IRendererList m_renderers;
      VolumeView *parent;
    };

    typedef boost::shared_ptr<Settings> SettingsPtr;

  public:
    explicit VolumeView(const EspinaFactory *factory,
                        ViewManager* viewManager,
                        bool additionalScrollBars = false,
                        QWidget* parent = 0);
    virtual ~VolumeView();

    virtual void reset();
    virtual void centerViewOn(Nm center[3], bool);
    void setCameraFocus(const Nm center[3]);

  public slots: //Needed to interact with renderers
    virtual void updateView();

  public:
    virtual void resetCamera();

    virtual void addChannel   (ChannelPtr channel);
    virtual void removeChannel(ChannelPtr channel);
    virtual bool updateChannelRepresentation(ChannelPtr channel, bool render = true);
    virtual void updateChannelRepresentations(ChannelList list = ChannelList());

    virtual void addSegmentation   (SegmentationPtr seg);
    virtual void removeSegmentation(SegmentationPtr seg);
    virtual bool updateSegmentationRepresentation(SegmentationPtr seg, bool render = true);
    virtual void updateSegmentationRepresentations(SegmentationList list = SegmentationList());

    virtual void addWidget   (EspinaWidget *widget);
    virtual void removeWidget(EspinaWidget *widget);

    virtual void addActor   (vtkProp3D *actor);
    virtual void removeActor(vtkProp3D *actor);

    virtual void setCursor(const QCursor& cursor);
    virtual void eventPosition(int& x, int& y);
    virtual ISelector::PickList pick(ISelector::PickableItems filter,
                                   ISelector::DisplayRegionList regions);
    virtual void worldCoordinates(const QPoint& displayPos,
                                  double worldCoordinatesc[3])
    { Q_ASSERT(false); }
    virtual void setSelectionEnabled(bool enabe){}

    virtual vtkRenderWindow* renderWindow();
    virtual vtkRenderer* mainRenderer();

    SettingsPtr settings() {return m_settings;}

    void changePlanePosition(PlaneType, Nm);
    void addRendererControls(IRendererSPtr renderer);
    void removeRendererControls(const QString name);

    void showCrosshairs(bool) {};
    virtual void updateSelection(){}

    virtual void forceRender(SegmentationList updatedSegs = SegmentationList());

  public slots:
    void countEnabledRenderers(bool);
    /// Update Selected Items
    virtual void updateSelection(ViewManager::Selection selection, bool render);
    void resetView();

  signals:
    void channelSelected(ChannelPtr);
    void segmentationSelected(SegmentationPtr, bool);
    void centerChanged(Nm, Nm, Nm);

  protected:
    void selectPickedItems(int x, int y, bool append);

    virtual void updateChannelsOpactity(){}

  private:
    struct ChannelState
    {
      double brightness;
      double contrast;
      double opacity;
      QColor stain;
      bool   visible;

      GraphicalRepresentationSList representations;
    };

    struct SegmentationState
    {
      Nm         depth;
      QColor     color;
      bool       highlited;
      OutputSPtr output;
      bool       visible;

      GraphicalRepresentationSList representations;
    };

    void setupUI();
    void buildControls();
    void updateRenderersButtons();
    void updateScrollBarsLimits();

  protected slots:
    virtual bool eventFilter(QObject* caller, QEvent* e);
    virtual void scrollBarMoved(int);

    void exportScene();
    void takeSnapshot();

  private:
    ViewManager *m_viewManager;

    // GUI
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QVTKWidget  *m_view;
    QPushButton m_snapshot;
    QPushButton m_export;
    QPushButton m_zoom;
    vtkSmartPointer<vtkRenderer> m_renderer;

    // Optional elements only visible in Segmentation Information dialog
    QHBoxLayout *m_additionalGUI;
    QScrollBar  *m_axialScrollBar;
    QScrollBar  *m_coronalScrollBar;
    QScrollBar  *m_sagittalScrollBar;
    bool m_additionalScrollBars;

    SettingsPtr m_settings;

    Nm m_center[3];
    unsigned int m_numEnabledRenders;
    unsigned int m_numEnabledSegmentationRenders;
    unsigned int m_numEnabledChannelRenders;
    ColorEngine *m_colorEngine;
    QMap<EspinaWidget *, vtkAbstractWidget *> m_widgets;
    QMap<QPushButton *, IRendererSPtr> m_renderers;
    IRendererSList   m_itemRenderers;

    QMap<ChannelPtr,      ChannelState>      m_channelStates;
    QMap<SegmentationPtr, SegmentationState> m_segmentationStates;

    vtkSmartPointer<vtkPropPicker> m_meshPicker;

    friend class GraphicalRepresentation;
  };

} // namespace EspINA

#endif // VOLUMEVIEW_H
