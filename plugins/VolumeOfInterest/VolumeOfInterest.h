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
#ifndef VOLUMEOFINTEREST_H
#define VOLUMEOFINTEREST_H

#include <QActionGroup>

#include <selection/PixelSelector.h>
#include <QSharedPointer>

class ActionSelector;
class QAction;
class VOISelector;

/// Volume Of Interest Plugin
class VolumeOfInterest 
: public QActionGroup
{
  Q_OBJECT
public:
  explicit VolumeOfInterest(QObject* parent);
  virtual ~VolumeOfInterest();

protected slots:
  void changeVOISelector(QAction *action);
  void defineVOI(SelectionHandler::MultiSelection msel);
  void cancelVOI();

private:
  void buildVOIs();

//   void addVOI(QAction *action, IVOI *voi);
// 
//   void buildSubPipeline(Product* input, EspinaParamList args);

private:
  ActionSelector *m_VOI;
  QSharedPointer<PixelSelector> m_selector;

//   VolumeOfInterestPreferences *m_preferences;

  static const int SliceOffset = 1;
};

#endif// VOLUMEOFINTEREST_H