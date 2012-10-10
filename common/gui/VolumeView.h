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

#include "ViewManager.h"
#include "EspinaRenderView.h"

#include <pluginInterfaces/Renderer.h>
#include <common/EspinaTypes.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <QPushButton>

class vtkAbstractWidget;
class QVTKWidget;

//Forward declaration
class Channel;
class ColorEngine;
class IViewWidget;
class QHBoxLayout;
class QToolButton;
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
    explicit Settings(const QString prefix=QString(), VolumeView *parent=NULL);

    void setRenderers(QList< Renderer* > values);
    QList<Renderer *> renderers() const;

  private:
    QList<Renderer *> m_renderers;
    VolumeView *parent;
  };

  typedef QSharedPointer<Settings> SettingsPtr;

public:
  explicit VolumeView(ViewManager *vm, QWidget* parent = 0);
  virtual ~VolumeView(){}

  void centerViewOn(Nm center[3]/*nm*/);
  void setCameraFocus(const Nm center[3]);

  virtual void updateView();
  virtual void resetCamera();

  virtual void addChannel(Channel *channel);
  virtual void removeChannel(Channel *channel);
  virtual bool updateChannel(Channel *channel);

  virtual void addSegmentation(Segmentation *seg);
  virtual void removeSegmentation(Segmentation *seg);
  virtual bool updateSegmentation(Segmentation* seg);

  virtual void addWidget(EspinaWidget *widget);
  virtual void removeWidget(EspinaWidget *widget);

  virtual void addPreview(vtkProp* preview){}
  virtual void removePreview(vtkProp* preview){}

  virtual void setCursor(const QCursor& cursor);
  virtual void eventPosition(int& x, int& y);
  virtual IPicker::PickList pick(IPicker::PickableItems filter,
                                 IPicker::DisplayRegionList regions);
  virtual vtkRenderWindow* renderWindow();

  SettingsPtr settings() {return m_settings;}

  void changePlanePosition(PlaneType, Nm);
  void addRendererControls(Renderer *);
  void removeRendererControls(const QString name);

  void updateSegmentationRepresentations();
  virtual void updateSelection(){}

public slots:
  void countEnabledRenderers(bool);
  /// Update Selected Items
  virtual void updateSelection(ViewManager::Selection selection);

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
  vtkSmartPointer<vtkRenderer> m_renderer;

  SettingsPtr m_settings;

  Nm m_center[3];
  unsigned int m_numEnabledRenders;
  ColorEngine *m_colorEngine;
  QList<vtkAbstractWidget *> m_widgets;

  QList<Segmentation *> m_segmentations;
  QList<ModelItem*> m_addedItems;
  QList<Renderer *> m_itemRenderers;
};

#endif // VOLUMEVIEW_H
