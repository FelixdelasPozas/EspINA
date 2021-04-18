/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "IssueProperty.h"

// Qt
#include <QWidget>

using namespace ESPINA;

//--------------------------------------------------------------------
IssueProperty::IssueProperty(const QString &warning, const QString &suggestion, QWidget* parent, QPixmap icon)
: QWidget{parent}
{
  setupUi(this);

  m_warning_label->setText(warning);
  auto font = m_warning_label->font();
  font.setBold(true);
  m_warning_label->setFont(font);

  m_suggestion_label->setText(suggestion);

  if (icon.isNull())
  {
    m_icon_label->hide();
  }
  else
  {
    m_icon_label->setPixmap(icon);
  }
}
