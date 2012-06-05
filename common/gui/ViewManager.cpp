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


#include "ViewManager.h"

#include <QDebug>

#include "common/gui/EspinaView.h"
#include "common/gui/SliceView.h"
// #include "gui/ViewFrame.h"

#include <QDockWidget>
#include <QLayout>
#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>

#include <pqApplicationCore.h>
#include <pqServerManagerObserver.h>

//----------------------------------------------------------------------------
ViewManager::ViewManager()
{
}

//----------------------------------------------------------------------------
ViewManager::~ViewManager()
{
}

//----------------------------------------------------------------------------
EspinaView *ViewManager::createView(QMainWindow *window, const QString& layout)
{
//   if (layout == "squared")
//     m_currentView = createSquaredLayout(window);
//   else
//     m_currentView = createDefaultLayout(window);

  Q_ASSERT(false);
  return NULL;
}

//----------------------------------------------------------------------------
EspinaView *ViewManager::createDefaultLayout(QMainWindow* window)
{
  return NULL;
//   return new DefaultEspinaView(window, "Segmentation");
}

//----------------------------------------------------------------------------
EspinaView* ViewManager::createSquaredLayout(QMainWindow* window)
{
  SliceView *volView = new SliceView();
  SliceView *xyView = new SliceView();

  QSplitter *upperViews = new QSplitter(Qt::Horizontal);
  upperViews->addWidget(volView);
  upperViews->addWidget(xyView);

  SliceView *yzView = new SliceView();
  SliceView *xzView = new SliceView();
  
  QSplitter *lowerViews = new QSplitter(Qt::Horizontal);
  lowerViews->addWidget(yzView);
  lowerViews->addWidget(xzView);

  QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
  mainSplitter->addWidget(upperViews);
  mainSplitter->addWidget(lowerViews);

  window->setCentralWidget(mainSplitter);
  return NULL;
}
