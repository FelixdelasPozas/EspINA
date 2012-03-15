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

//TODO: Show bounding regions in volume view
//TODO: Show bounding regions in slice view

#ifndef COUNTINGREGION_H
#define COUNTINGREGION_H

#include <common/gui/EspinaDockWidget.h>
#include <common/model/EspinaModel.h>

#include <QStandardItemModel>

// Forward declaration
class CountingRegion
: public EspinaDockWidget
{
  Q_OBJECT
  class GUI;
public:
  static const QString ID;

public:
  explicit CountingRegion(QWidget* parent);
  virtual ~CountingRegion();

protected:
  void createAdaptiveRegion(double inclusion[3], double exclusion[3]);
  void createRectangularRegion(double inclusion[3], double exclusion[3]);

protected slots:
  void clearBoundingRegions();
  /// Creates a bounding region on the current focused/active
  /// sample and update all their segmentations counting regions
  /// extension discarting those that are out of the region
  void createBoundingRegion();
  void removeSelectedBoundingRegion();
  void sampleChanged(Sample *sample);
  void showInfo(const QModelIndex& index);
  void saveRegionDescription();

private:
  GUI *m_gui;
  QStandardItemModel m_regionModel;
  QSharedPointer<EspinaModel> m_espinaModel;
};

#endif // COUNTINGREGION_H
