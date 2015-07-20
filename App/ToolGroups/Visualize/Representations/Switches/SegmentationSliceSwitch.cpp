/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/ListUtils.hxx>
#include <ToolGroups/Visualize/Representations/Switches/SegmentationSliceSwitch.h>

using namespace ESPINA;

const QString SEGMENTATION_SLICE_ENABLED_KEY = "Enabled";
const QString SEGMENTATION_SLICE_OPACITY_KEY = "Opacity";

//----------------------------------------------------------------------------
SegmentationSliceSwitch::SegmentationSliceSwitch(const QString &id,
                                                 GUI::Representations::RepresentationManagerSPtr manager,
                                                 std::shared_ptr<SegmentationSlicePoolSettings>  settings,
                                                 ViewTypeFlags                                   supportedViews,
                                                 Timer                                          &timer,
                                                 Support::Context                               &context)
: BasicRepresentationSwitch(id, manager, supportedViews, timer, context)
, m_settings{settings}
{
  initWidgets();

  connect(settings.get(), SIGNAL(modified()),
          this,           SLOT(onSettingsModified()));
}

//----------------------------------------------------------------------------
SegmentationSliceSwitch::~SegmentationSliceSwitch()
{
}

//----------------------------------------------------------------------------
void SegmentationSliceSwitch::onOpacityChanged()
{
  if(m_opacityWidget->value() != m_settings->opacity() * 100)
  {
    auto opacity = m_opacityWidget->value() / 100.0;

    m_settings->setOpacity(opacity);

    auto items = m_manager->pools().first()->sources();

    invalidateRepresentations(items);
  }
}

//----------------------------------------------------------------------------
void SegmentationSliceSwitch::onSettingsModified()
{
  if(m_opacityWidget->value() != m_settings->opacity() * 100)
  {
    m_opacityWidget->setValue(m_settings->opacity()*100);
  }
}

//----------------------------------------------------------------------------
void SegmentationSliceSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto checked = settings->value(SEGMENTATION_SLICE_ENABLED_KEY, true).toBool();
  auto opacity = settings->value(SEGMENTATION_SLICE_OPACITY_KEY, 0.6).toDouble();

  m_settings->setOpacity(opacity);
  setChecked(checked);
}

//----------------------------------------------------------------------------
void SegmentationSliceSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(SEGMENTATION_SLICE_ENABLED_KEY, isChecked());
  settings->setValue(SEGMENTATION_SLICE_OPACITY_KEY, m_settings->opacity());
}

//----------------------------------------------------------------------------
void SegmentationSliceSwitch::initWidgets()
{
  m_opacityWidget = new GUI::Widgets::NumericalInput();
  m_opacityWidget->setLabelText(tr("Opacity"));
  m_opacityWidget->setMinimum(0);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setSliderTracking(false);
  m_opacityWidget->setValue(m_settings->opacity()*100);
  m_opacityWidget->setSpinBoxVisibility(false);

  addSettingsWidget(m_opacityWidget);

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(onOpacityChanged()));
}

