/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "LineEditWithClear.h"

// Qt
#include <QStyle>
#include <QToolButton>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
LineEditWithClear::LineEditWithClear(QWidget *parent)
: QLineEdit(parent)
{
  auto height     = sizeHint().height();
  auto buttonSize = height - 4;

  m_button = new QToolButton(this);
  m_button->setIcon(QIcon(":/espina/cancel.png"));
  m_button->setToolTip(tr("Clear text"));
  m_button->setCursor(Qt::ArrowCursor);
  m_button->setStyleSheet("QToolButton { border: none; padding: 2px}");
  m_button->setFixedSize(buttonSize, buttonSize);
  m_button->hide();

  auto frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(buttonSize - frameWidth));
  setMinimumHeight(height);

  connect(m_button, SIGNAL(clicked()),
          this,     SLOT(clear()));

  connect(this, SIGNAL(textChanged(const QString&)),
          this, SLOT(updateClearButton(const QString&)));
}

//--------------------------------------------------------------------
void LineEditWithClear::resizeEvent(QResizeEvent *)
{
  auto frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  m_button->move(width() - m_button->width() - frameWidth, 2);
}

//--------------------------------------------------------------------
void LineEditWithClear::updateClearButton(const QString& text)
{
  m_button->setVisible(!text.isEmpty());
}
