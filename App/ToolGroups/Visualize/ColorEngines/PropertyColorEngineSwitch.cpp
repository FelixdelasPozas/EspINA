/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "PropertyColorEngineSwitch.h"

#include <GUI/ColorEngines/PropertyColorEngine.h>
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Utils/Format.h>
#include <GUI/Utils/ColorRange.h>
#include <QComboBox>
#include <QLabel>
#include <QLayout>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Utils::Format;
using namespace ESPINA::Support;

//-----------------------------------------------------------------------------
PropertyColorEngineSwitch::PropertyColorEngineSwitch(Context& context)
: ColorEngineSwitch(std::make_shared<PropertyColorEngine>(context), ":espina/color_engine_switch_property.svg", context)
{
  createPropertySelector();

  createColorRange();
}

//-----------------------------------------------------------------------------
void PropertyColorEngineSwitch::createPropertySelector()
{
  auto label = new QLabel(tr("Color by:"));
  m_property = new QLabel(createLink(valueColorEngine()->measure()));

  m_property->setOpenExternalLinks(false);

  connect(m_property, SIGNAL(linkActivated(QString)),
          this,       SLOT(changeProperty()));

  addSettingsWidget(label);
  addSettingsWidget(m_property);
}

//-----------------------------------------------------------------------------
void PropertyColorEngineSwitch::createColorRange()
{
  auto colorBar = new ColorBar(valueColorEngine()->colorRange());

  colorBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  colorBar->setFixedHeight(20);
  colorBar->setMinimumWidth(80);
  colorBar->setMaximumWidth(80);

  addSettingsWidget(colorBar);
}

//-----------------------------------------------------------------------------
PropertyColorEngine* PropertyColorEngineSwitch::valueColorEngine() const
{
  return dynamic_cast<PropertyColorEngine *>(colorEngine().get());
}

//-----------------------------------------------------------------------------
void PropertyColorEngineSwitch::changeProperty()
{
  auto available = availableInformation(getFactory());

  auto selection = InformationSelector::GroupedInfo();

  InformationSelector propertySelector(available, selection, tr("Select property to color by"));

  if (propertySelector.exec() == QDialog::Accepted)
  {
    m_extensionType  = selection.keys().first();
    m_informationTag = selection[m_extensionType].first();

    valueColorEngine()->setMeasure(m_informationTag, 0, 10000);

    m_property->setText(createLink(m_informationTag));
  }
}

