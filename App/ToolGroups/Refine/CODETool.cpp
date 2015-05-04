/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "CODETool.h"

#include <Support/Widgets/Styles.h>

#include <QHBoxLayout>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support::Widgets;

//------------------------------------------------------------------------
CODETool::CODETool(const QString& icon, const QString& tooltip)
: m_toggle       {new QAction(QIcon(icon), tooltip, this)}
, m_nestedOptions{new QWidgetAction(this)}
, m_radius       {new NumericalInput()}
{
  m_toggle->setCheckable(true);

  initOptionWidgets();
}

//------------------------------------------------------------------------
QList<QAction *> CODETool::actions() const
{
  QList<QAction *> actions;

  actions << m_toggle << m_nestedOptions;

  return actions;
}

//------------------------------------------------------------------------
void CODETool::abortOperation()
{
}

//------------------------------------------------------------------------
void CODETool::toggleToolWidgets(bool toggle)
{
  m_toggle->setChecked(toggle);
  m_nestedOptions->setVisible(toggle);

  emit toggled(toggle);
}

//------------------------------------------------------------------------
void CODETool::onToolEnabled(bool enabled)
{
  m_toggle->setEnabled(enabled);
  m_nestedOptions->setEnabled(enabled);

  if(m_toggle->isChecked() && !enabled)
  {
    toggleToolWidgets(false);
  }
}

//------------------------------------------------------------------------
void CODETool::initOptionWidgets()
{
  connect(m_toggle, SIGNAL(toggled(bool)),
          this,     SLOT(toggleToolWidgets(bool)));

  m_radius->setLabelText(tr("Radius"));
  m_radius->setMinimum(1);
  m_radius->setMaximum(99);
  m_radius->setSliderVisibility(false);

  auto apply = Tool::createToolButton(":/espina/tick.png", tr("Apply"));

  connect(apply, SIGNAL(clicked(bool)),
          this,  SIGNAL(applyClicked()));

  auto widget = new QWidget();
  auto layout = new QHBoxLayout();

  layout->addWidget(m_radius);
  layout->addWidget(apply);

  widget->setLayout(layout);

  Styles::setNestedStyle(widget);

  m_nestedOptions->setDefaultWidget(widget);
  m_nestedOptions->setVisible(false);
}
