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
#include <GUI/Widgets/Styles.h>
#include <ToolGroups/Visualize/Representations/Switches/SegmentationVolumetricSwitch.h>

// Qt
#include <QPushButton>

using namespace ESPINA;

const QString SEGMENTATION_VOLUMETRIC_ENABLED_KEY    = "Enabled";
const QString SEGMENTATION_VOLUMETRIC_SWITCH_USE_GPU = "Use GPU";

//----------------------------------------------------------------------------
SegmentationVolumetricSwitch::SegmentationVolumetricSwitch(GUI::Representations::RepresentationManagerSPtr cpuManager,
                                                           GUI::Representations::RepresentationManagerSPtr gpuManager,
                                                           ViewTypeFlags                                   supportedViews,
                                                           Timer                                          &timer,
                                                           Support::Context                               &context)
: RepresentationSwitch("VolumetricSwitch", cpuManager->icon(), cpuManager->description(), timer, context)
, m_cpuManager        {cpuManager}
, m_gpuManager        {gpuManager}
, m_flags             {supportedViews}
{
  initWidgets();
}

//----------------------------------------------------------------------------
SegmentationVolumetricSwitch::~SegmentationVolumetricSwitch()
{
}

//----------------------------------------------------------------------------
ViewTypeFlags SegmentationVolumetricSwitch::supportedViews()
{
  return m_flags;
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::showRepresentations(TimeStamp t)
{
  if(m_gpuEnable->isChecked())
  {
    m_gpuManager->show(t);
  }
  else
  {
    m_cpuManager->show(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::hideRepresentations(TimeStamp t)
{
  if(m_gpuEnable->isChecked())
  {
    m_gpuManager->hide(t);
  }
  else
  {
    m_cpuManager->hide(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  auto enabled = settings->value(SEGMENTATION_VOLUMETRIC_ENABLED_KEY, false).toBool();
  auto gpuEnabled = settings->value(SEGMENTATION_VOLUMETRIC_SWITCH_USE_GPU, true).toBool();

  setChecked(enabled);
  m_gpuEnable->setChecked(gpuEnabled);
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(SEGMENTATION_VOLUMETRIC_ENABLED_KEY, isChecked());
  settings->setValue(SEGMENTATION_VOLUMETRIC_SWITCH_USE_GPU, m_gpuEnable->isChecked());
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::onModeChanged(bool check)
{
  if(!isChecked()) return;

  auto t = getViewState().timer().increment();

  if(check)
  {
    m_cpuManager->hide(t);
    m_gpuManager->show(t);
  }
  else
  {
    m_cpuManager->show(t);
    m_gpuManager->hide(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationVolumetricSwitch::initWidgets()
{
  m_gpuEnable = GUI::Widgets::Styles::createToolButton(QIcon(":/espina/gpuchip.png"), tr("Use GPU acceleration."));
  m_gpuEnable->setCheckable(true);

  connect(m_gpuEnable, SIGNAL(toggled(bool)),
          this,        SLOT(onModeChanged(bool)));

  addSettingsWidget(m_gpuEnable);
}
