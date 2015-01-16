/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "CrosshairsRenderSwitch2D.h"
#include <Support/Widgets/Tool.h>

#include <QIcon>
#include <QPushButton>
#include <QHBoxLayout>

using namespace ESPINA;

//-----------------------------------------------------------------------------
CrosshairsRenderSwitch2D::CrosshairsRenderSwitch2D(ViewManagerSPtr viewManager)
: m_viewManager{viewManager}
, m_crosshairsVisible{false}
{
}


//-----------------------------------------------------------------------------
QWidget *CrosshairsRenderSwitch2D::widget()
{
  auto showCrosshais = Tool::createToolButton(QIcon(":espina/show_crosshairs2D.svg"), tr("Show Crosshairs 2D"));

  showCrosshais->setCheckable(true);

  connect(showCrosshais, SIGNAL(toggled(bool)),
          this,          SLOT(setCrosshairsVisibility(bool)), Qt::QueuedConnection);


  return showCrosshais;
}


//-----------------------------------------------------------------------------
ViewTypeFlags CrosshairsRenderSwitch2D::supportedViews()
{
  ViewTypeFlags flags;

  flags = ViewType::VIEW_2D;

  return flags;
}

//-----------------------------------------------------------------------------
void CrosshairsRenderSwitch2D::setCrosshairsVisibility(bool visibile)
{
  m_crosshairsVisible = visibile;
  m_viewManager->setCrosshairVisibility(visibile);
}