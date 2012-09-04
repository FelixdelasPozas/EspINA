/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef DEFAULTESPINAVIEW_H
#define DEFAULTESPINAVIEW_H

#include <common/gui/EspinaView.h>
#include <common/EspinaTypes.h>
#include <gui/VolumeView.h>

class VolumeViewSettingsPanel;
class SliceViewSettingsPanel;
class ColorEngine;
class Segmentation;
// Forward-declaration
class SliceView;
class VolumeView;

class DefaultEspinaView
: public EspinaView
{
  Q_OBJECT

  struct Widgtes
  {
    SliceWidget  *xy;
    SliceWidget  *yz;
    SliceWidget  *xz;
    vtkAbstractWidget *vol;
  };

  class SettingsPanel;

public:
  explicit DefaultEspinaView(QMainWindow* parent, const QString activity = QString());
  virtual ~DefaultEspinaView();

  virtual void createViewMenu(QMenu* menu);
  virtual void restoreLayout();
  virtual void saveLayout();

  virtual void forceRender();
  virtual void resetCamera();

  virtual QSize sizeHint() const;

  virtual void slicingStep(Nm steps[3]);
  virtual void setSlicingStep(Nm steps[3]);

  virtual void addWidget(EspinaWidget* widget);
  virtual void removeWidget(EspinaWidget* widget);

//   virtual void addRepresentation(pqOutputPort *oport, QColor color);
//   virtual void removeRepresentation(pqOutputPort *oport);

  virtual ISettingsPanel* settingsPanel();

public slots:
  virtual void setColorEngine(ColorEngine *engine);
  virtual void showCrosshair(bool visible);
  virtual void switchPreprocessing();
  virtual void showSegmentations(bool visible);
  virtual void showThumbnail(bool visible);

  virtual void setCameraFocus(const Nm focus[3]);

  virtual void setCrosshairPoint(Nm x, Nm y, Nm z, bool force = false);
  virtual void setSliceSelectors(SliceView::SliceSelectors selectors);
  virtual void changePlanePosition(PlaneType, Nm);

protected:
  void addChannelRepresentation(Channel *channel);
  void removeChannelRepresentation(Channel *channel);
  bool updateChannel(Channel *channel);

  void addSegmentation(Segmentation *seg);
  void removeSegmentation(Segmentation *seg);
  bool updateSegmentation(Segmentation *seg);


protected slots:
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
  virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

  void setFitToSlices(bool fit);
  void setRulerVisibility(bool visible);

  void selectFromSlice(double slice, PlaneType plane);
  void selectToSlice(double slice, PlaneType plane);

  void channelSelected(Channel *channel);
  void segmentationSelected(Segmentation *seg, bool append);

  void updateSceneRanges();

  void initSliceView(SliceView *view);

private:
  bool m_showProcessing;
  bool m_showSegmentations;
  Nm   m_slicingStep[3];

  ColorEngine *m_colorEngine;
  SliceView  *xyView, *yzView, *xzView;
  VolumeView *volView;
  QDockWidget *volDock, *yzDock, *xzDock;
  QAction     *m_showRuler, *m_showThumbnail;

  QList<Channel *> m_channels;
  QMap<EspinaWidget *, Widgtes> m_widgets;
};

class DefaultEspinaView::SettingsPanel
: public ISettingsPanel
{
public:
  explicit SettingsPanel(SliceView::SettingsPtr xy,
		        SliceView::SettingsPtr yz,
		        SliceView::SettingsPtr xz,
		        VolumeView::SettingsPtr vol);

  virtual const QString shortDescription() {return tr("View");}
  virtual const QString longDescription() {return tr("%1 Settings").arg(shortDescription());}
  virtual const QIcon icon() {return QIcon(":/espina/show_all.svg");}

  virtual void acceptChanges();

  virtual bool modified() const;

  virtual ISettingsPanel* clone();

private:
  Nm m_slicingStep;
  SliceView::SettingsPtr m_xy, m_yz, m_xz;
  SliceViewSettingsPanel *m_xyPanel, *m_yzPanel, *m_xzPanel;
  VolumeView::SettingsPtr m_vol;
  VolumeViewSettingsPanel *m_volPanel;
};

#endif // DEFAULTESPINAVIEW_H
