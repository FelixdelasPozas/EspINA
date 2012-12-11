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


#ifndef COLORENGINESETTINGS_H
#define COLORENGINESETTINGS_H

#include <QMenu>

#include <Core/ColorEngines/IColorEngine.h>
#include <Core/ColorEngines/MultiColorEngine.h>

#include <QMap>

class ColorEngine;
class ViewManager;

class ColorEngineMenu
: public QMenu
{
  Q_OBJECT
public:
  explicit ColorEngineMenu(ViewManager *vm, const QString &title, QWidget *parent = 0);
  virtual ~ColorEngineMenu();

  ColorEnginePtr engine() const {return m_engine;}

  void addColorEngine(const QString &title, ColorEnginePtr engine);
  void restoreUserSettings();

protected slots:
  void setColorEngine(QAction *action);

signals:
  void colorEngineChanged(ColorEnginePtr);

private:
  ViewManager *m_viewManager;
  QSharedPointer<MultiColorEngine> m_engine;
  QMap<QAction *, ColorEnginePtr>  m_availableEngines;
};

#endif // COLORENGINESETTINGS_H
