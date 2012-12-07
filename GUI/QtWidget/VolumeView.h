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

class EspinaFactory;
class vtkAbstractWidget;
class QVTKWidget;

//Forward declaration
class Channel;
class ColorEngine;
class IViewWidget;
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class Renderer;
class Sample;
class Segmentation;

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
                      const QString prefix=QString(),
                      VolumeView *parent=NULL);

    void setRenderers(QList< Renderer* > values);
    QList<Renderer *> renderers() const;

  private:
    QList<Renderer *> m_renderers;
    VolumeView *parent;
  };

  typedef QSharedPointer<Settings> SettingsPtr;

public:
  explicit VolumeView(const EspinaFactory *factory,
                      ViewManager* vm,
                      QWidget* parent = 0);
  virtual ~VolumeView();

  void centerViewOn(Nm *center, bool);
  void setCameraFocus(const Nm center[3]);

public slots: //Needed to interact with renderers
  virtual void updateView();

public:
  virtual void resetCamera();

  virtual void addChannel(Channel *channel);
  virtual void removeChannel(Channel *channel);
  virtual bool updateChannel(Channel *channel);

  virtual void addSegmentation(Segmentation *seg);
  virtual void removeSegmentation(Segmentation *seg);
  virtual bool updateSegmentation(Segmentation* seg);

  virtual void addWidget(EspinaWidget *widget);
  virtual void removeWidget(EspinaWidget *widget);

  virtual void addPreview(vtkProp3D *preview){}
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
  void addRendererControls(Renderer *);
  void removeRendererControls(const QString name);

  void showCrosshairs(bool) {};
  void updateSegmentationRepresentations();

public slots:
  void countEnabledRenderers(bool);
  /// Update Selected Items
  virtual void updateSelection(ViewManager::Selection selection, bool render);
  void resetView();

signals:
  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);

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
  QMap<QPushButton *, Renderer *> m_renderers;

  QList<Segmentation *> m_segmentations;
  QList<ModelItem*> m_addedItems;
  QList<Renderer *> m_itemRenderers;
};

#endif // VOLUMEVIEW_H
