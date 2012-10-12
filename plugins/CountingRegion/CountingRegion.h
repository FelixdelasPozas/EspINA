/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef COUNTINGREGION_H
#define COUNTINGREGION_H

#include <common/pluginInterfaces/IDockWidget.h>
#include <common/pluginInterfaces/IColorEngineProvider.h>

#include <common/model/EspinaModel.h>
#include <common/EspinaTypes.h>

#include <QStandardItemModel>

// Forward declaration
class BoundingRegion;
class Channel;

/// Counting Region Plugin
class CountingRegion
: public IDockWidget
, public IColorEngineProvider
{
  Q_OBJECT
  Q_INTERFACES(IDockWidget IColorEngineProvider)
  class GUI;
public:
  static const QString ID;

  typedef QList<BoundingRegion *> RegionList;

public:
  explicit CountingRegion(QWidget* parent=NULL);
  virtual ~CountingRegion();

  virtual void initDockWidget(EspinaModel* model,
                              QUndoStack* undoStack,
                              ViewManager* viewManager);

  virtual EngineList colorEngines();

  virtual void reset();

  void createAdaptiveRegion(Channel *channel,
                            Nm inclusion[3],
                            Nm exclusion[3]);
  void createRectangularRegion(Channel *channel,
                               Nm inclusion[3],
                               Nm exclusion[3]);

  RegionList regions() const {return m_regions;}

protected slots:
  void clearBoundingRegions();
  /// Creates a bounding region on the current focused/active
  /// sample and update all their segmentations counting regions
  /// extension discarting those that are out of the region
  void createBoundingRegion();
  void removeSelectedBoundingRegion();
  void channelChanged(Channel *channel);
  void showInfo(const QModelIndex& index);
  void saveRegionDescription();


signals:
  void regionCreated(BoundingRegion *);
  void regionRemoved(BoundingRegion *);

private:
  GUI *m_gui;
  QStandardItemModel m_regionModel;
  EspinaModel *m_espinaModel;
  ViewManager *m_viewManager;
  RegionList m_regions;
};

#endif // COUNTINGREGION_H
