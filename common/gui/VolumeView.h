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
class vtkVolumeView;

/// VolumeView
class VolumeView
: public QWidget
{
  Q_OBJECT
public:
  class Settings;
  typedef QSharedPointer<Settings> SettingsPtr;

public:
  explicit VolumeView(QWidget* parent = 0);
  virtual ~VolumeView(){}

  void centerViewOn(double center[3]/*nm*/);
  void setCameraFocus(double center[3]);
  void resetCamera();

  void addChannelRepresentation(Channel *channel);
  bool updateChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);

  void addSegmentationRepresentation(Segmentation *seg);
  void removeSegmentationRepresentation(Segmentation *seg);
  bool updateSegmentationRepresentation(Segmentation* seg);

//TODO:   void addWidget(pq3DWidget *widget);
//TODO:   void removeWidget(pq3DWidget *widget);

  void setColorEngine(ColorEngine *engine){m_colorEngine = engine;}
  SettingsPtr settings() {return m_settings;}

public slots:
  void forceRender();

signals:
  void channelSelected(Channel *);
  void segmentationSelected(Segmentation *, bool);

protected:
  void init();
  double suggestedChannelOpacity();
  void selectPickedItems(bool append);

private:
//   void selectSegmentations(int x, int y, int z);

protected slots:
  virtual bool eventFilter(QObject* caller, QEvent* e);

  void exportScene();
  void takeSnapshot();

  void buildControls();

private:
  struct Representation
  {
    //pqOutputPort *outport;
    //vtkSMRepresentationProxy *proxy;
    bool visible;
    bool selected;
    QColor color;
  };

  vtkVolumeView *m_view;

  // GUI
  QVBoxLayout *m_mainLayout;
  QHBoxLayout *m_controlLayout;
  QVTKWidget  *m_viewWidget;
  QPushButton m_snapshot;
  QPushButton m_export;

  SettingsPtr m_settings;

  double m_center[3];
  ColorEngine *m_colorEngine;

  QMap<Segmentation *, Representation> m_segmentations;
};

class VolumeView::Settings
{
  const QString RENDERERS;
public:
  typedef QSharedPointer<Renderer> RendererPtr;
  explicit Settings(const QString prefix=QString());

  void setRenderers(QList< Renderer* > values);
  QList<RendererPtr> renderers() const;

private:
  QList<RendererPtr> m_renderers;
};

#endif // VOLUMEVIEW_H