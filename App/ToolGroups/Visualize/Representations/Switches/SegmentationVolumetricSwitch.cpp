/*
 Copyright (C) 2017  Felix de las Pozas Alvarez

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
#include <ToolGroups/Visualize/Representations/Switches/SegmentationVolumetricSwitch.h>
#include <GUI/Widgets/Styles.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;

const QString VOLUMETRIC_GPU_ENABLED_KEY = QObject::tr("Volumetric GPU rendering");

//--------------------------------------------------------------------
SegmentationVolumetricSwitch::SegmentationVolumetricSwitch(GUI::Representations::RepresentationManagerSPtr gpuManager,
                                                           GUI::Representations::RepresentationManagerSPtr cpuManager,
                                                           Support::Context                               &context)
: RepresentationSwitch{"SegmentationVolumetricSwitch", QIcon(":espina/display_volume.svg"), QObject::tr("Display Segmentation Volume"), context}
, m_gpuManager        {gpuManager}
, m_cpuManager        {cpuManager}
{
  initParameterWidgets();
}

//--------------------------------------------------------------------
SegmentationVolumetricSwitch::~SegmentationVolumetricSwitch()
{
  if(isChecked())
  {
    if(m_gpuOptionButton->isChecked())
    {
      disconnectGPUManagerPools();
    }
    else
    {
      disconnectCPUManagerPools();
    }
  }
}

//--------------------------------------------------------------------
ViewTypeFlags SegmentationVolumetricSwitch::supportedViews()
{
  return ViewTypeFlags{ViewType::VIEW_3D};
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::showRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  if(!m_gpuOptionButton->isChecked())
  {
    connectCPUManagerPools();
    m_cpuManager->show(frame);
  }
  else
  {
    connectGPUManagerPools();
    m_gpuManager->show(frame);
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::hideRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  if(!m_gpuOptionButton->isChecked())
  {
    m_cpuManager->hide(frame);
    disconnectCPUManagerPools();
  }
  else
  {
    m_gpuManager->hide(frame);
    disconnectGPUManagerPools();
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto gpuEnabled = settings->value(VOLUMETRIC_GPU_ENABLED_KEY, false).toBool();

  if(m_gpuOptionButton->isChecked() != gpuEnabled)
  {
    m_gpuOptionButton->setChecked(gpuEnabled);
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);
  settings->setValue(VOLUMETRIC_GPU_ENABLED_KEY, m_gpuOptionButton->isChecked());
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::onModeChanged(bool enabled)
{
  auto frame = getViewState().createFrame();

  if(enabled)
  {
    m_cpuManager->blockSignals(true);
    m_cpuManager->hide(frame);
    disconnectCPUManagerPools();
    m_cpuManager->blockSignals(false);

    connectGPUManagerPools();
    m_gpuManager->show(frame);
  }
  else
  {
    m_gpuManager->blockSignals(true);
    m_gpuManager->hide(frame);
    disconnectGPUManagerPools();
    m_gpuManager->blockSignals(false);

    connectCPUManagerPools();
    m_cpuManager->show(frame);
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::initParameterWidgets()
{
  m_gpuOptionButton = createToolButton(QIcon(":espina/GPU.svg"), QObject::tr("Enable GPU Rendering"));
  m_gpuOptionButton->setCheckable(true);

  connect(m_gpuOptionButton, SIGNAL(toggled(bool)),
          this,              SLOT(onModeChanged(bool)));

  addSettingsWidget(m_gpuOptionButton);
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::connectGPUManagerPools()
{
  for(auto pool: m_gpuManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::disconnectGPUManagerPools()
{
  for(auto pool: m_gpuManager->pools())
  {
    disconnect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
               this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::connectCPUManagerPools()
{
  for(auto pool: m_cpuManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::disconnectCPUManagerPools()
{
  for(auto pool: m_cpuManager->pools())
  {
    disconnect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
               this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//--------------------------------------------------------------------
void SegmentationVolumetricSwitch::invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto manager = (m_gpuOptionButton->isChecked() ? m_gpuManager : m_cpuManager);

  for(auto pool: manager->pools())
  {
    pool->invalidateRepresentations(items, frame);
  }
}
