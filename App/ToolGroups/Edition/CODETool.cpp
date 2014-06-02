/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "CODETool.h"

using namespace EspINA;

//------------------------------------------------------------------------
CODETool::CODETool(const QString& icon, const QString& tooltip)
: m_toggle(new QAction(this))
, m_radius(new SpinBoxAction(this))
, m_apply(new QAction(this))
{
  m_toggle->setIcon(QIcon(icon));
  m_toggle->setToolTip(tooltip);
  m_toggle->setCheckable(true);

  m_radius->setLabelText(tr("Radius"));
  m_radius->setSpinBoxMaximum(99);

  m_apply->setIcon(QIcon(":/espina/tick.png"));

  toggleToolWidgets(false);

  connect(m_toggle, SIGNAL(toggled(bool)),
          this,     SLOT(toggleToolWidgets(bool)));

  connect(m_apply, SIGNAL(triggered(bool)),
          this,    SIGNAL(applyClicked()));
}

//------------------------------------------------------------------------
QList<QAction *> CODETool::actions() const
{
  QList<QAction *> actions;

  actions << m_toggle << m_radius << m_apply;

  return actions;
}

//------------------------------------------------------------------------
void CODETool::setEnabled(bool enabled)
{
  m_toggle->setEnabled(enabled);
  m_radius->setEnabled(enabled);
  m_apply ->setEnabled(enabled);

  if(m_toggle->isChecked() && !enabled)
    toggleToolWidgets(false);

  //if (m_toggle->isChecked() && !enabled)
  //{
  //  m_toggle->setChecked(false);
  //  m_radius->setVisible(false);
  //  m_apply ->setVisible(false);
  //}
}

//------------------------------------------------------------------------
void CODETool::toggleToolWidgets(bool toggle)
{
  m_toggle->setChecked(toggle);
  m_radius->setVisible(toggle);
  m_apply ->setVisible(toggle);

  emit toggled(toggle);
}
