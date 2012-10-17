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

#include <QToolBar>

#include "common/tools/PixelSelector.h"
#include "common/EspinaTypes.h"

#include <QMap>

class ActionSelector;
class IVOI;
class ViewManager;
class QAction;

/// Volume Of Interest Plugin
class VolumeOfInterest 
: public QToolBar
{
  Q_OBJECT
public:
  explicit VolumeOfInterest(ViewManager *vm, QWidget *parent=NULL);
  virtual ~VolumeOfInterest();

protected slots:
  void changeVOI(QAction *action);
  void cancelVOI();

  void setBorderFrom(double pos/*nm*/, PlaneType plane);
  void setBorderTo  (double pos/*nm*/, PlaneType plane);

private:
  void buildVOIs();

private:
  ViewManager     *m_viewManager;
  ActionSelector  *m_voiSelector;
  QMap<QAction *, IVOI *> m_vois;
};

#endif// VOLUMEOFINTEREST_H
