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


//----------------------------------------------------------------------------
// File:    ViewManager.h
// Purpose: Create new views and keep them valid whenever the server
//          connection changes.
//----------------------------------------------------------------------------
#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>

class QMainWindow;
class EspinaView;
class QWidget;
class ViewFrame;

class ViewManager : public QObject
{
  Q_OBJECT
public:
  explicit ViewManager();
  ~ViewManager();

  void setCurrentView(EspinaView *view) {m_currentView = view;}
  EspinaView *currentView() {return m_currentView;}
  EspinaView *createView(QMainWindow *window, const QString &layout = QString()); //NOTE: Should be move into the factory?
//   void saveView(const QString &layout) const;
//   void restoreView(const QString &layout);

protected:
  EspinaView *createDefaultLayout(QMainWindow *window);
  EspinaView *createSquaredLayout(QMainWindow *window);

  EspinaView *m_currentView;
};

#endif // VIEWMANAGER_H
