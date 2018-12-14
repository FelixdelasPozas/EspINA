/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// Plugin
#include <Representations/RepresentationSwitch.h>
#include <Representations/RepresentationManager3D.h>

using namespace ESPINA;

const QString OPACITY_KEY = "CountingFrameOpacity";

//------------------------------------------------------------------------
CF::CFRepresentationSwitch::CFRepresentationSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                                                   Support::Context                               &context)
: BasicRepresentationSwitch{"CD3DSwitch", manager, ViewType::VIEW_3D, context}
{
  initWidget();
}

//------------------------------------------------------------------------
CF::CFRepresentationSwitch::~CFRepresentationSwitch()
{
}

//------------------------------------------------------------------------
void CF::CFRepresentationSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto opacity = settings->value(OPACITY_KEY, 0.7).toFloat() * 100;
  onOpacityChanged(opacity);
  m_opacityWidget->setValue(opacity);
}

//------------------------------------------------------------------------
void CF::CFRepresentationSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  auto manager = std::dynamic_pointer_cast<RepresentationManager3D>(this->m_manager);
  if(manager)
  {
    settings->setValue(OPACITY_KEY, manager->opacity());
  }
}

//------------------------------------------------------------------------
void CF::CFRepresentationSwitch::onOpacityChanged(int value)
{
  auto manager = std::dynamic_pointer_cast<CF::RepresentationManager3D>(m_manager);
  if(manager)
  {
    manager->setOpacity(static_cast<float>(value)/100.0);
    getViewState().refresh();
  }
}

//------------------------------------------------------------------------
void CF::CFRepresentationSwitch::initWidget()
{
  m_opacityWidget = new GUI::Widgets::NumericalInput();
  m_opacityWidget->setLabelText(tr("Opacity"));
  m_opacityWidget->setMinimum(0);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setSliderTracking(false);
  m_opacityWidget->setValue(70);
  m_opacityWidget->setSpinBoxVisibility(false);
  m_opacityWidget->setToolTip(tr("%1 representation's opacity.").arg(m_manager->name()));

  addSettingsWidget(m_opacityWidget);

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(onOpacityChanged(int)));
}
