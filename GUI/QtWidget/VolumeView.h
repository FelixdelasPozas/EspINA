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

// EspINA
#include "Core/EspinaTypes.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include "GUI/Renderers/Renderer.h"
#include "GUI/Representations/GraphicalRepresentation.h"
#include "GUI/ViewManager.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

// Qt
#include <QPushButton>

class vtkAbstractWidget;
class QVTKWidget;

//Forward declaration
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QScrollBar;

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

    virtual GraphicalRepresentationSPtr cloneRepresentation(GraphicalRepresentationSPtr prototype);

  public slots: //Needed to interact with renderers
    virtual void updateView();
    virtual void updateSelection(){};

  public:
    virtual void resetCamera();

    virtual void addChannel   (ChannelPtr channel);
    virtual void removeChannel(ChannelPtr channel);
    virtual bool updateChannelRepresentation(ChannelPtr channel, bool render = true);

    virtual bool updateSegmentationRepresentation(SegmentationPtr seg, bool render = true);

    virtual void addWidget   (EspinaWidget *widget);
    virtual void removeWidget(EspinaWidget *widget);

    virtual ISelector::PickList pick(ISelector::PickableItems filter,
                                   ISelector::DisplayRegionList regions);

    virtual void setSelectionEnabled(bool enable){}

    SettingsPtr settings() {return m_settings;}

    void changePlanePosition(PlaneType, Nm);

    void addRendererControls(IRendererSPtr renderer);
    void removeRendererControls(const QString name);

    void showCrosshairs(bool) {};
    virtual void worldCoordinates(const QPoint &displayPos, double worldPos[3]) {};

  public slots:
    void updateEnabledRenderersCount(bool);

  signals:
    void centerChanged(Nm, Nm, Nm);

  protected:
    void selectPickedItems(int x, int y, bool append);

    virtual void updateChannelsOpactity(){}

  private:
    void setupUI();
    void buildControls();
    void updateRenderersControls();
    void updateScrollBarsLimits();

  protected slots:
    virtual bool eventFilter(QObject* caller, QEvent* e);
    virtual void scrollBarMoved(int);

    void exportScene();
    void takeSnapshot();

  private:
    // GUI
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QPushButton m_snapshot;
    QPushButton m_export;
    QPushButton m_zoom;

    // GUI elements only visible in Segmentation Information dialog
    QHBoxLayout *m_additionalGUI;
    QScrollBar  *m_axialScrollBar;
    QScrollBar  *m_coronalScrollBar;
    QScrollBar  *m_sagittalScrollBar;
    bool m_additionalScrollBars;

    SettingsPtr m_settings;

    Nm m_center[3];
    QMap<EspinaWidget *, vtkAbstractWidget *> m_widgets;
    IRendererSList   m_itemRenderers;
  };

} // namespace EspINA

#endif // VOLUMEVIEW_H
