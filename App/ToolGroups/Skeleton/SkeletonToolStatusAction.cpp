/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonToolStatusAction.h"

// Qt
#include <QIcon>
#include <QWidget>
#include <QHBoxLayout>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SkeletonToolStatusAction::SkeletonToolStatusAction(QObject *parent)
  : QWidgetAction{parent}
  , m_label      {nullptr}
  , m_createIcon {nullptr}
  , m_modifyIcon {nullptr}
  , m_enabled    {true}
  {
  }
  
  //-----------------------------------------------------------------------------
  SkeletonToolStatusAction::~SkeletonToolStatusAction()
  {
  }

  //-----------------------------------------------------------------------------
  QWidget* SkeletonToolStatusAction::createWidget(QWidget* parent)
  {
    QWidget*     widget = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout();

    widget->setLayout(layout);

    m_label      = new QLabel(tr("Tool status:"));
    m_createIcon = new QLabel();
    m_modifyIcon = new QLabel();

    m_label     ->setEnabled(m_enabled);
    m_createIcon->setEnabled(m_enabled);
    m_modifyIcon->setEnabled(m_enabled);

    layout->addWidget(m_label);
    layout->addWidget(m_createIcon);
    layout->addWidget(m_modifyIcon);

    setStatus(SkeletonWidget::Status::READY_TO_CREATE);

    return widget;
  }

  //-----------------------------------------------------------------------------
  void SkeletonToolStatusAction::reset()
  {
    setStatus(SkeletonWidget::Status::READY_TO_CREATE);
  }

  //-----------------------------------------------------------------------------
  void SkeletonToolStatusAction::setEnabled(bool value)
  {
    m_enabled = value;

    if (nullptr != m_label)
    {
      m_label->setEnabled(value);
      m_createIcon->setEnabled(value);
      m_modifyIcon->setEnabled(value);
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonToolStatusAction::setStatus(SkeletonWidget::Status status)
  {
    if(!m_createIcon || !m_modifyIcon) return;

    QIcon createIcon = QIcon(":espina/skelCreate.png");
    QIcon modifyIcon = QIcon(":espina/skelModify.png");

    switch(status)
    {
      case SkeletonWidget::Status::READY_TO_CREATE:
        m_createIcon->setToolTip(tr("Ready to create an skeleton. Push the 'Tab' key to start creating an skeleton and push again to stop."));
        m_createIcon->setPixmap(createIcon.pixmap(createIcon.actualSize(QSize(22,22)), QIcon::Normal));
        m_modifyIcon->setToolTip(tr("The skeleton hasn't been created yet."));
        m_modifyIcon->setPixmap(modifyIcon.pixmap(modifyIcon.actualSize(QSize(22,22)), QIcon::Disabled));
        break;
      case SkeletonWidget::Status::CREATING:
        m_createIcon->setPixmap(createIcon.pixmap(createIcon.actualSize(QSize(22,22)), QIcon::Normal));
        m_modifyIcon->setPixmap(modifyIcon.pixmap(modifyIcon.actualSize(QSize(22,22)), QIcon::Disabled));
        break;
      case SkeletonWidget::Status::READY_TO_EDIT:
        m_createIcon->setToolTip(tr("Ready to modify the skeleton. Push the 'Tab' key to start creating an skeleton and push again to stop."));
        m_createIcon->setPixmap(createIcon.pixmap(createIcon.actualSize(QSize(22,22)), QIcon::Normal));
        m_modifyIcon->setToolTip(tr("Push the 'Alt' key over a node to delete it. Use mouse left button over a node to translate it."));
        m_modifyIcon->setPixmap(modifyIcon.pixmap(modifyIcon.actualSize(QSize(22,22)), QIcon::Normal));
        break;
      case SkeletonWidget::Status::EDITING:
        m_createIcon->setPixmap(createIcon.pixmap(createIcon.actualSize(QSize(22,22)), QIcon::Disabled));
        m_modifyIcon->setPixmap(modifyIcon.pixmap(modifyIcon.actualSize(QSize(22,22)), QIcon::Normal));
        break;
      default:
        break;
    }
  }

} // namespace ESPINA
