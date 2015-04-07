/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "CamerasMenu.h"

// Qt
#include <QDateTime>
#include <QInputDialog>
#include <QLineEdit>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  CamerasMenu::CamerasMenu(ViewStateSPtr viewState, QWidget *parent)
  : QMenu(parent)
  , m_viewState(viewState)
  {
    m_save = new QAction(QString("Save..."), this);
    addAction(m_save);

    m_load = new QMenu(QString("Load"), this);
    addMenu(m_load);
    m_load->setEnabled(false);

    m_clear = new QAction(QString("Clear"), this);
    addAction(m_clear);
    m_clear->setEnabled(false);

    setTitle("Camera positions");
    connect(this, SIGNAL(triggered(QAction*)), this, SLOT(activate(QAction *)), Qt::QueuedConnection);
  }

  //-----------------------------------------------------------------------------
  CamerasMenu::~CamerasMenu()
  {
    delete m_clear;
    delete m_save;
    delete m_load;
  }

  //-----------------------------------------------------------------------------
  void CamerasMenu::loadPositions(CameraPositionsList list)
  {
    Q_ASSERT(m_cameraPositions.empty());

    m_cameraPositions = list;
    for (auto snapshot: m_cameraPositions)
    {
      auto action = new QAction(snapshot.id, this);
      m_load->addAction(action);
    }

    m_clear->setEnabled(!list.empty());
    m_load->setEnabled(!list.empty());
  }
  //-----------------------------------------------------------------------------
  void CamerasMenu::clearPositions()
  {
    m_cameraPositions.clear();

    for(auto action: m_load->actions())
      m_load->removeAction(action);

    m_load->setEnabled(false);
    m_clear->setEnabled(false);
  }

  //-----------------------------------------------------------------------------
  void CamerasMenu::save()
  {
    CameraPositions state;
    QDateTime time = QDateTime::currentDateTime();
    state.id = time.toString();

    bool ok;
    QString text = QInputDialog::getText(this,
                                         tr("Enter name"),
                                         tr("Name:"),
                                         QLineEdit::Normal,
                                         state.id, &ok);

    if (!(ok && !text.isEmpty())) return;

    state.id = text;

    // TODO Decide where we should pass the view set to save
//     for(auto view: m_viewManager->renderViews())
//     {
//       state.states << view->cameraState();
//     }


    m_cameraPositions << state;

    if (m_load->isEnabled() == false)
    {
      m_load->setEnabled(true);
      m_clear->setEnabled(true);
    }

    m_load->addAction(new QAction(state.id, this));
  }

  //-----------------------------------------------------------------------------
  void CamerasMenu::activate(QAction *action)
  {
    if (action == m_clear)
    {
      clearPositions();
    }
    else if(action == m_save)
    {
      save();
    }
    else
    {
      if (m_load->actions().contains(action))
      {
        auto snapshot = m_cameraPositions.at(m_load->actions().indexOf(action));

        // TODO restore view state
//         for(auto state: snapshot.states)
//         {
//           for(auto view: m_viewManager->renderViews())
//           {
//             view->setCameraState(state);
//           }
//         }
      }
    }
  }

} /* namespace ESPINA */
