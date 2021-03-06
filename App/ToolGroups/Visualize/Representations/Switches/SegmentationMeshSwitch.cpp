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
#include <ToolGroups/Visualize/Representations/Switches/SegmentationMeshSwitch.h>
#include <GUI/Representations/Settings/SegmentationMeshPoolSettings.h>

using namespace ESPINA;

const QString SEGMENTATION_MESH_SMOOTH_VALUE_KEY = "Smooth Value";

//----------------------------------------------------------------------------
SegmentationMeshSwitch::SegmentationMeshSwitch(GUI::Representations::RepresentationManagerSPtr meshManager,
                                               GUI::Representations::RepresentationManagerSPtr smoothedMeshManager,
                                               std::shared_ptr<SegmentationMeshPoolSettings>   settings,
                                               Support::Context                               &context)
: RepresentationSwitch ("SegmentationMeshSwitch", meshManager->icon(), meshManager->description(), context)
, m_meshManager        {meshManager}
, m_smoothedMeshManager{smoothedMeshManager}
, m_settings           {settings}
, m_smoothEnabled      {false}
{
  initWidgets();
}

//----------------------------------------------------------------------------
SegmentationMeshSwitch::~SegmentationMeshSwitch()
{
  if(!m_smoothEnabled && m_smoothedMeshManager->isActive())
  {
    disconnectMeshManagersPools();
  }
  else
  {
    if(m_meshManager->isActive())
    {
      disconnectSmoothMeshManagersPools();
    }
  }
}

//----------------------------------------------------------------------------
ViewTypeFlags SegmentationMeshSwitch::supportedViews() const
{
  return ViewTypeFlags{ViewType::VIEW_3D};
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::showRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  if(m_smooth->value() != 0)
  {
    connectSmoothMeshManagersPools();
    m_smoothedMeshManager->show(frame);
  }
  else
  {
    connectMeshManagersPools();
    m_meshManager->show(frame);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::hideRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  if(m_smooth->value() != 0)
  {
    m_smoothedMeshManager->hide(frame);
    disconnectSmoothMeshManagersPools();
  }
  else
  {
    m_meshManager->hide(frame);
    disconnectMeshManagersPools();
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto smoothValue = settings->value(SEGMENTATION_MESH_SMOOTH_VALUE_KEY, 0).toInt();

  m_smooth->setValue(smoothValue);
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(SEGMENTATION_MESH_SMOOTH_VALUE_KEY, m_smooth->value());
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::onSmoothChanged(int value)
{
  if(m_settings->smoothValue() != value)
  {
    auto smoothEnabled = (value != 0);

    if (smoothEnabled != m_smoothEnabled)
    {
      m_smoothEnabled = smoothEnabled;
      switchManagers();
    }

    m_settings->setSmoothValue(value);

    if(m_smoothEnabled)
    {
      const ViewItemAdapterList items = m_smoothedMeshManager->pools().first()->sources();

      invalidateRepresentations(items);
    }
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::initWidgets()
{
  m_smooth = new GUI::Widgets::NumericalInput();
  m_smooth->setLabelText("Smooth");
  m_smooth->setSpinBoxVisibility(false);
  m_smooth->setSliderTracking(false);
  m_smooth->setValue(m_settings->smoothValue());
  m_smooth->setToolTip(tr("Segmentation surface smoothing."));

  connect(m_smooth, SIGNAL(valueChanged(int)),
          this,     SLOT(onSmoothChanged(int)));

  addSettingsWidget(m_smooth);
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::switchManagers()
{
  if(!isChecked()) return;

  auto frame = getViewState().createFrame();

  if(m_smoothEnabled)
  {
    m_meshManager->blockSignals(true);
    m_meshManager->hide(frame);
    disconnectMeshManagersPools();
    m_meshManager->blockSignals(false);

    connectSmoothMeshManagersPools();
    m_smoothedMeshManager->show(frame);
  }
  else
  {
    m_smoothedMeshManager->blockSignals(true);
    m_smoothedMeshManager->hide(frame);
    disconnectSmoothMeshManagersPools();
    m_smoothedMeshManager->blockSignals(false);

    connectMeshManagersPools();
    m_meshManager->show(frame);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto manager = (m_smoothEnabled ? m_smoothedMeshManager : m_meshManager);

  for(auto pool: manager->pools())
  {
    pool->invalidateRepresentations(items, frame);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::connectMeshManagersPools()
{
  for(auto pool: m_meshManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::connectSmoothMeshManagersPools()
{
  for(auto pool: m_smoothedMeshManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::disconnectMeshManagersPools()
{
  for(auto pool: m_meshManager->pools())
  {
    disconnect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
               this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::disconnectSmoothMeshManagersPools()
{
  for(auto pool: m_smoothedMeshManager->pools())
  {
    disconnect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
               this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}
