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
#include <GUI/Representations/Managers/ConnectionsManager.h>
#include <ToolGroups/Visualize/Representations/Switches/SegmentationConnectionsSwitch.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Representations::Settings;

const QString SIZE_KEY = "Connections Representation Size";

//--------------------------------------------------------------------
SegmentationConnectionsSwitch::SegmentationConnectionsSwitch(RepresentationManagerSPtr manager, ConnectionSettingsSPtr settings, Support::Context& context)
: BasicRepresentationSwitch{"ConnectionsSwitch", manager, manager->supportedViews(), context}
, m_settings{settings}
{
  connect(m_settings.get(), SIGNAL(modified()), this, SLOT(onSettingsModified()));

  initializeParameterWidgets();

  onSizeValueChanged(m_settings->connectionSize());
}

//--------------------------------------------------------------------
void SegmentationConnectionsSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto sizeValue = settings->value(SIZE_KEY, 5).toInt();
  m_sizeWidget->setValue(sizeValue);
  m_settings->setConnectionSize(sizeValue);

  restoreCheckedState(settings);
}

//--------------------------------------------------------------------
void SegmentationConnectionsSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(SIZE_KEY, m_settings->connectionSize());
}

//--------------------------------------------------------------------
void SegmentationConnectionsSwitch::onSizeValueChanged(int value)
{
  if(value != m_settings->connectionSize())
  {
    m_settings->setConnectionSize(value);

    std::dynamic_pointer_cast<ConnectionsManager>(m_manager)->setRepresentationSize(value);
  }
}

//--------------------------------------------------------------------
void SegmentationConnectionsSwitch::onSettingsModified()
{
  auto size = m_settings->connectionSize();

  if(size != m_sizeWidget->value())
  {
    m_sizeWidget->blockSignals(true);
    m_sizeWidget->setValue(size);
    m_sizeWidget->blockSignals(false);

    std::dynamic_pointer_cast<ConnectionsManager>(m_manager)->setRepresentationSize(size);
  }
}

//--------------------------------------------------------------------
void SegmentationConnectionsSwitch::initializeParameterWidgets()
{
  m_sizeWidget = new NumericalInput();
  m_sizeWidget->setLabelText(tr("Size"));
  m_sizeWidget->setSpinBoxVisibility(false);
  m_sizeWidget->setSliderVisibility(true);
  m_sizeWidget->setMinimum(1);
  m_sizeWidget->setMaximum(30);
  m_sizeWidget->setValue(m_settings->connectionSize());

  connect(m_sizeWidget, SIGNAL(valueChanged(int)), this, SLOT(onSizeValueChanged(int)));

  addSettingsWidget(m_sizeWidget);
}

