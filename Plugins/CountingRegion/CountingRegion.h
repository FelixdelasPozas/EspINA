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

#include <Core/Interfaces/IDockWidget.h>
#include <Core/Interfaces/IColorEngineProvider.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaTypes.h>

#include <QStandardItemModel>

// Forward declaration
class BoundingRegion;
class Channel;
class CountingRegionChannelExtension;

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

public slots:
  void resetState();

protected slots:
  void clearBoundingRegions();
  /// Update UI depending on regions' selected row
  void updateUI(int row);
  void showInfo(BoundingRegion *region);
  void updateSegmentations();
  /// Creates a bounding region on the current focused/active
  /// sample and update all their segmentations counting regions
  /// extension discarting those that are out of the region
  void createBoundingRegion();
  /// Update active bounding region's margins
  void updateBoundingMargins();
  void removeSelectedBoundingRegion();
  void channelChanged(Channel *channel);
  void saveRegionDescription();

private:
  /// Find margin values which discard all segmentations that
  /// touch the channel margins
  void computeOptimalMargins(Channel *channel,
                             Nm inclusion[3],
                             Nm exclusion[3]);

  /// Return inclusion margins definded by the UI
  void inclusionMargins(double values[3]);
  /// Return exclusion margins definded by the UI
  void exclusionMargins(double values[3]);

  void registerRegion(CountingRegionChannelExtension *ext,
                      BoundingRegion *region);

signals:
  void regionCreated(BoundingRegion *);
  void regionRemoved(BoundingRegion *);

private:
  GUI *m_gui;

  EspinaModel *m_espinaModel;
  ViewManager *m_viewManager;

  RegionList m_regions;
  BoundingRegion *m_activeRegion;

  static const int NUM_FIXED_ROWS = 2;
};

#endif // COUNTINGREGION_H
