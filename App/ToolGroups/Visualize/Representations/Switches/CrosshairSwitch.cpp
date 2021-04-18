/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/ToolGroups/Visualize/Representations/Switches/CrosshairSwitch.h>
#include <GUI/Representations/Managers/CrosshairManager.h>
#include <GUI/Widgets/Styles.h>

using ESPINA::GUI::Representations::Managers::CrosshairManager;

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

const QString INTERSECTIONS_KEY = "ShowIntersections";

//--------------------------------------------------------------------
CrosshairSwitch::CrosshairSwitch(GUI::Representations::RepresentationManagerSPtr manager, ViewTypeFlags supportedViews, Support::Context& context)
: BasicRepresentationSwitch("DisplayCrosshair", manager, supportedViews, context)
, m_intersectionButton{nullptr}
{
  if(supportedViews.testFlag(ViewType::VIEW_3D)) createSettingsWidgets();
}

//--------------------------------------------------------------------
void CrosshairSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  if(m_intersectionButton)
  {
    auto enabled = settings->value(INTERSECTIONS_KEY, false).toBool();

    m_intersectionButton->setChecked(enabled);
  }
}

//--------------------------------------------------------------------
void CrosshairSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  if(m_intersectionButton)
  {
    settings->setValue(INTERSECTIONS_KEY, m_intersectionButton->isChecked());
  }
}

//--------------------------------------------------------------------
void CrosshairSwitch::setShowPlaneIntersections(const bool value)
{
  if(supportedViews().testFlag(ViewType::VIEW_3D))
  {
    auto cManager = std::dynamic_pointer_cast<CrosshairManager>(m_manager);
    if(cManager)
    {
      cManager->setShowIntersections(value);
    }
  }
}

//--------------------------------------------------------------------
const bool CrosshairSwitch::showPlaneIntersections() const
{
  return m_intersectionButton && m_intersectionButton->isChecked();
}

//--------------------------------------------------------------------
void CrosshairSwitch::onIntersectionButtonPressed(bool unused)
{
  if(!m_intersectionButton) return;

  auto enabled = m_intersectionButton->isChecked();
  auto manager = std::dynamic_pointer_cast<CrosshairManager>(m_manager);
  if(manager)
  {
    manager->setShowIntersections(enabled);
  }
}

//--------------------------------------------------------------------
void CrosshairSwitch::createSettingsWidgets()
{
  m_intersectionButton = Styles::createToolButton(QIcon(":/espina/display_crosshairs_intersections.svg"), tr("Show/Hide plane intersection lines"));
  connect(m_intersectionButton, SIGNAL(clicked(bool)), this, SLOT(onIntersectionButtonPressed(bool)));
  m_intersectionButton->setCheckable(true);
  m_intersectionButton->setChecked(false);

  addSettingsWidget(m_intersectionButton);
}
