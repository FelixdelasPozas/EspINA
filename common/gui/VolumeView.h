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

#include <QAbstractItemView>
#include <QPushButton>
#include <pluginInterfaces/Renderer.h>
#include <common/EspinaTypes.h>
#include <selection/SelectionManager.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

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
: public QWidget
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
  explicit VolumeView(QWidget* parent = 0);
  virtual ~VolumeView(){}

  void centerViewOn(Nm center[3]/*nm*/);
  void setCameraFocus(const Nm center[3]);
  void resetCamera();

  void addChannelRepresentation(Channel *channel);
  bool updateChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  bool updateSegmentationRepresentation(Segmentation* seg);

  void addWidget(vtkAbstractWidget *widget);
  void removeWidget(vtkAbstractWidget *widget);

  void setColorEngine(ColorEngine *engine)
  {
    m_colorEngine = engine;
    updateSegmentationRepresentations();
  }

  SettingsPtr settings() {return m_settings;}

  void changePlanePosition(PlaneType, Nm);
  void addRendererControls(Renderer *);
  void removeRendererControls(Renderer *);

protected:
  void updateSegmentationRepresentations();

public slots:
  void forceRender();
  void countEnabledRenderers(bool);
  /// Update Selected Items
  virtual void updateSelection(SelectionManager::Selection selection);

signals:
  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);

protected:
  void init();
  double suggestedChannelOpacity();
  void selectPickedItems(bool append);

private:
//   void selectSegmentations(int x, int y, int z);
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

  // GUI
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QVTKWidget  *m_viewWidget;
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
