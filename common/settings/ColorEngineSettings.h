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

#include <QObject>
#include <QMap>

class QActionGroup;
class ColorEngine;
class QAction;
class QMenu;

const QString COLOR_ENGINE("ColorEngine");

class ColorEngineSettings : public QObject
{
  Q_OBJECT
public:
  explicit ColorEngineSettings();

  ColorEngine *engine() const {return m_engine;}
  QMenu *availableEngines();

public slots:
  void setColorEngine(ColorEngine *engine);

protected slots:
  void setColorEngine(QAction *engine);

signals:
  void colorEngineChanged();

private:
  ColorEngine  *m_engine;
  QActionGroup *m_actions;
  QMap<QAction *, ColorEngine *> m_availableEngines;
};

#endif // COLORENGINESETTINGS_H
