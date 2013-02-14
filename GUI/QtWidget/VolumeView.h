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

namespace EspINA
{
  class ColorEngine;
  class IViewWidget;
  /// VolumeView
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

    typedef QSharedPointer<Settings> SettingsPtr;

  public:
    explicit VolumeView(const EspinaFactory *factory,
                        ViewManager* viewManager,
                        QWidget* parent = 0);
    virtual ~VolumeView();

    virtual void reset();

    void centerViewOn(Nm *center, bool);
    void setCameraFocus(const Nm center[3]);

    public slots: //Needed to interact with renderers
      virtual void updateView();

  public:
    virtual void resetCamera();

    virtual void addChannel   (ChannelPtr channel);
    virtual void removeChannel(ChannelPtr channel);
    virtual bool updateChannel(ChannelPtr channel);

    virtual void addSegmentation   (SegmentationPtr seg);
    virtual void removeSegmentation(SegmentationPtr seg);
    virtual bool updateSegmentation(SegmentationPtr seg);

    virtual void addWidget   (EspinaWidget *widget);
    virtual void removeWidget(EspinaWidget *widget);

    virtual void addPreview   (vtkProp3D *preview){}
    virtual void removePreview(vtkProp3D *preview){}

    virtual void setCursor(const QCursor& cursor);
    virtual void eventPosition(int& x, int& y);
    virtual IPicker::PickList pick(IPicker::PickableItems filter,
                                   IPicker::DisplayRegionList regions);
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
    virtual void updateSegmentationRepresentations(SegmentationList list = SegmentationList());
    virtual void updateChannelRepresentations(ChannelList list = ChannelList());
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

  protected:
    void selectPickedItems(bool append);

  private:
    //   void selectSegmentations(int x, int y, int z);
    void setupUI();
    void buildControls();

  protected slots:
    virtual bool eventFilter(QObject* caller, QEvent* e);

    void exportScene();
    void takeSnapshot();

  private:
    struct Representation
    {
      bool visible;
      bool selected;
      QColor color;
    };

    ViewManager *m_viewManager;

    // GUI
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QVTKWidget  *m_view;
    QPushButton m_snapshot;
    QPushButton m_export;
    QPushButton m_zoom;
    vtkSmartPointer<vtkRenderer> m_renderer;

    SettingsPtr m_settings;

    Nm m_center[3];
    unsigned int m_numEnabledRenders;
    unsigned int m_numEnabledSegmentationRenders;
    ColorEngine *m_colorEngine;
    QMap<EspinaWidget *, vtkAbstractWidget *> m_widgets;
    QMap<QPushButton *, IRendererSPtr> m_renderers;

    SegmentationList m_segmentations;
    ModelItemList    m_addedItems;
    IRendererSList   m_itemRenderers;
  };

} // namespace EspINA

#endif // VOLUMEVIEW_H
