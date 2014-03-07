/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

// EspINA
#include "CamerasMenu.h"

// Qt
#include <QDateTime>
#include <QInputDialog>
#include <QLineEdit>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  CamerasMenu::CamerasMenu(ViewManagerSPtr vm, QWidget *parent)
  : QMenu(parent)
  , m_viewManager(vm)
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
    if (ok && !text.isEmpty())
      state.id = text;
    else
      return;

    for(auto view: m_viewManager->renderViews())
      state.states << view->visualState();

    m_cameraPositions << state;
    auto action = new QAction(state.id, this);

    if (m_load->isEnabled() == false)
    {
      m_load->setEnabled(true);
      m_clear->setEnabled(true);
    }

    m_load->addAction(action);
  }
  
  //-----------------------------------------------------------------------------
  void CamerasMenu::activate(QAction *action)
  {
    if (action == m_clear)
      clearPositions();
    else
      if(action == m_save)
        save();
      else
      {
        if (m_load->actions().contains(action))
        {
          struct CameraPositions snapshot = m_cameraPositions.at(m_load->actions().indexOf(action));
          for(auto state: snapshot.states)
            for(auto view: m_viewManager->renderViews())
              view->setVisualState(state);
        }
      }
  }

} /* namespace EspINA */
