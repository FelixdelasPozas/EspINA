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

#ifndef ESPINA_VIEW_3D_H
#define ESPINA_VIEW_3D_H

#include "GUI/View/RenderView.h"

// EspINA
#include "GUI/Representations/Renderers/Renderer.h"
#include "GUI/Representations/Representation.h"

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

  class EspinaGUI_EXPORT View3D
  : public RenderView
  {
    Q_OBJECT
  public:
    explicit View3D(bool additionalScrollBars = false,
                        QWidget* parent = 0);
    virtual ~View3D();

    virtual void reset();

    virtual void centerViewOn(const NmVector3& point, bool force = false);

    void setCameraFocus(const NmVector3& center);

    void setRenderers(RendererList values);

    RendererList renderers() const;

    virtual RepresentationSPtr cloneRepresentation(RepresentationSPtr prototype);

  public slots: //Needed to interact with renderers
    virtual void updateView();

    virtual void updateSelection(){};

  public:
    virtual void resetCamera();

    virtual void addChannel   (ChannelAdapterPtr channel);
    virtual void removeChannel(ChannelAdapterPtr channel);
    virtual bool updateRepresentation(ChannelAdapterPtr channel, bool render = true);

    virtual bool updateRepresentation(SegmentationAdapterPtr seg, bool render = true);

    virtual void addWidget   (EspinaWidget *widget);
    virtual void removeWidget(EspinaWidget *widget);

    virtual Selector::SelectionList pick(Selector::SelectionFlags filter, Selector::DisplayRegionList regions);

    virtual Selector::Selection select(Selector::SelectionFlags flags, Selector::SelectionMask mask) {/*TODO*/}

    void changePlanePosition(Plane, Nm);

    void addRendererControls(RendererSPtr renderer);

    void removeRendererControls(const QString name);

    void showCrosshairs(bool) {};

    virtual bool eventFilter(QObject* caller, QEvent* e);

  public slots:
    void updateEnabledRenderersCount(bool);

  signals:
    void centerChanged(NmVector3);

  protected:
    void selectPickedItems(int x, int y, bool append);

    virtual void updateChannelsOpactity(){}

  protected slots:

    virtual void scrollBarMoved(int);

    void exportScene();

    void onTakeSnapshot();

  private:
    void setupUI();

    void buildControls();

    void updateRenderersControls();

    void updateScrollBarsLimits();


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

    NmVector3 m_center;
    QMap<EspinaWidget *, vtkAbstractWidget *> m_widgets;
    RendererSList   m_renderers;
  };

} // namespace EspINA

#endif // ESPINA_VIEW_3D_H
