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
#ifndef SEEDGROWINGSEGMENTATION_H
#define SEEDGROWINGSEGMENTATION_H


#include <QActionGroup>
#include <plugins/FilterFactory.h>

#include <selection/SelectionHandler.h>
#include "SeedGrowSelector.h"

#include <QSharedPointer>

//Forward declarations
class ActionSelector;
class DefaultVOIAction;
class SeedGrowSegmentationFilter;
class ThresholdAction;


/// Seed Growing Segmenation Plugin
class SeedGrowSegmentation
: public QActionGroup
, public FilterFactory
{
  Q_OBJECT
public:
  SeedGrowSegmentation(QObject* parent);
  virtual ~SeedGrowSegmentation();

  virtual FilterPtr createFilter(const QString filter, const QString args);

protected slots:
  /// Wait for Seed Selection
  void waitSeedSelection(QAction *action);
  /// Abort current selection
  void abortSelection();
  void onSelectionAborted();
  /// Starts the segmentation filter putting using @msel as seed
  void startSegmentation(SelectionHandler::MultiSelection msel);
  void modifyLastFilter(int value);

signals:
//   void productCreated(Segmentation *);
  void selectionAborted(SelectionHandler *);

private:
  void addPixelSelector(QAction *action, SelectionHandler *handler);
  void buildSelectors();

private:
  ThresholdAction  *m_threshold;
  DefaultVOIAction *m_useDefaultVOI;
  ActionSelector   *m_segment;
  QMap<QAction *, SelectionHandler *> m_selectors;
  QSharedPointer<SeedGrowSelector> m_selector;
//   SeedGrowSegmentationSettings *m_preferences;
};

#endif// SEEDGROWINGSEGMENTATION_H