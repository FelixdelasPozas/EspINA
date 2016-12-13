/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Plugin
#include "ColorEngineSwitch.h"
#include "CountingFrameManager.h"

// Espina
#include <GUI/Widgets/NumericalInput.h>
#include <Core/Utils/ListUtils.hxx>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::CF;

const QString OPACITY_KEY = QString("Counting Frame Representation Opacity");

//-----------------------------------------------------------------------------
ColorEngineSwitch::ColorEngineSwitch(CountingFrameManager *manager, CountingFrameColorEngineSPtr engine, Support::Context& context)
: Support::Widgets::ColorEngineSwitch(engine, ":color_by_cf.svg", context)
, m_engine {engine}
, m_manager{manager}
{
  initWidgets();
  onCountingFrameNumberModified();

  connect(m_manager, SIGNAL(countingFrameCreated(CountingFrame *)), this, SLOT(onCountingFrameNumberModified()));
  connect(m_manager, SIGNAL(countingFrameDeleted(CountingFrame *)), this, SLOT(onCountingFrameNumberModified()));
}

//-----------------------------------------------------------------------------
void ColorEngineSwitch::initWidgets()
{
  auto opacity = new GUI::Widgets::NumericalInput();
  opacity->setLabelText(tr("Opacity"));
  opacity->setMinimum(0);
  opacity->setMaximum(100);
  opacity->setSliderTracking(false);
  opacity->setValue(m_engine->exlcusionOpacity()*100);
  opacity->setSpinBoxVisibility(false);
  opacity->setToolTip(tr("Colored representation's opacity."));

  addSettingsWidget(opacity);

  connect(opacity, SIGNAL(valueChanged(int)),
          this,    SLOT(onOpacityChanged(int)));
}

//-----------------------------------------------------------------------------
void ColorEngineSwitch::onOpacityChanged(int value)
{
  m_engine->setExclusionOpacity(value/100.0);

  auto segmentations = toRawList<ViewItemAdapter>(getModel()->segmentations());
  getViewState().invalidateRepresentationColors(segmentations);
}

//-----------------------------------------------------------------------------
void ColorEngineSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(OPACITY_KEY, m_engine->exlcusionOpacity());
}

//-----------------------------------------------------------------------------
void ColorEngineSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  m_engine->setExclusionOpacity(settings->value(OPACITY_KEY, 0.75).toDouble());
}

//-----------------------------------------------------------------------------
void ColorEngineSwitch::onCountingFrameNumberModified()
{
  auto enabled = m_manager->countingFrames().size() != 0;
  setEnabled(enabled);

  if(isChecked() && !enabled)
  {
    setChecked(false);
  }
}
