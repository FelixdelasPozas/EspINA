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
                                               ViewTypeFlags                                   supportedViews,
                                               Timer                                          &timer,
                                               Support::Context                               &context)
: RepresentationSwitch ("SegmentationMeshSwitch", meshManager->icon(), meshManager->description(), timer, context)
, m_meshManager        {meshManager}
, m_smoothedMeshManager{smoothedMeshManager}
, m_settings           {settings}
, m_flags              {supportedViews}
, m_smoothEnabled      {false}
{
  initWidgets();

  for(auto pool: meshManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }

  for(auto pool: smoothedMeshManager->pools())
  {
    connect(pool.get(), SIGNAL(taskStarted(TaskSPtr)),
            this,       SLOT(showTaskProgress(TaskSPtr)));
  }
}

//----------------------------------------------------------------------------
SegmentationMeshSwitch::~SegmentationMeshSwitch()
{
}

//----------------------------------------------------------------------------
ViewTypeFlags SegmentationMeshSwitch::supportedViews()
{
  return m_flags;
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::showRepresentations(TimeStamp t)
{
  if(m_smooth->value() != 0)
  {
    m_smoothedMeshManager->show(t);
  }
  else
  {
    m_meshManager->show(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::hideRepresentations(TimeStamp t)
{
  if(m_smooth->value() != 0)
  {
    m_smoothedMeshManager->hide(t);
  }
  else
  {
    m_meshManager->hide(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto smoothValue = settings->value(SEGMENTATION_MESH_SMOOTH_VALUE_KEY, 0).toInt();

  m_settings->setSmoothValue(smoothValue);
  auto smoothEnabled = (smoothValue != 0);

  if (m_smoothEnabled != smoothEnabled)
  {
    m_smoothEnabled = smoothEnabled;
    switchManagers();
  }
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

    m_settings->setSmoothValue(value);

    if (smoothEnabled != m_smoothEnabled)
    {
      m_smoothEnabled = smoothEnabled;
      switchManagers();
    }

    if(m_smoothEnabled)
    {
      ViewItemAdapterList items = m_smoothedMeshManager->pools().first()->sources();

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

  connect(m_smooth, SIGNAL(valueChanged(int)),
          this,     SLOT(onSmoothChanged(int)));

  addSettingsWidget(m_smooth);
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::switchManagers()
{
  if(!isChecked()) return;

  auto t = getViewState().timer().increment();

  if(m_smoothEnabled)
  {
    m_meshManager->hide(t);
    m_smoothedMeshManager->show(t);
  }
  else
  {
    m_smoothedMeshManager->hide(t);
    m_meshManager->show(t);
  }
}

//----------------------------------------------------------------------------
void SegmentationMeshSwitch::invalidateRepresentationsImplementation(ViewItemAdapterList items, TimeStamp t)
{
  auto manager = (m_smoothEnabled ? m_smoothedMeshManager : m_meshManager);

  for(auto pool: manager->pools())
  {
    pool->invalidateRepresentations(items, t);
  }
}
