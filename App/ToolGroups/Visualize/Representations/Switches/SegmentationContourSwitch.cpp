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
#include <ToolGroups/Visualize/Representations/Switches/SegmentationContourSwitch.h>

// Qt
#include <QComboBox>
#include <QLabel>

using namespace ESPINA;

const QString SEGMENTATION_CONTOUR_OPACITY_KEY = "Opacity";
const QString SEGMENTATION_CONTOUR_WIDTH_KEY   = "Width";
const QString SEGMENTATION_CONTOUR_PATTERN_KEY = "Pattern";

//----------------------------------------------------------------------------
SegmentationContourSwitch::SegmentationContourSwitch(GUI::Representations::RepresentationManagerSPtr  manager,
                                                     std::shared_ptr<SegmentationContourPoolSettings> settings,
                                                     ViewTypeFlags                                    supportedViews,
                                                     Timer                                           &timer,
                                                     Support::Context                                &context)
: BasicRepresentationSwitch("SegmentationContourSwitch", manager, supportedViews, timer, context)
, m_settings{settings}
{
  initWidgets();

  connect(settings.get(), SIGNAL(modified()),
          this,           SLOT(onSettingsModified()));
}

//----------------------------------------------------------------------------
ESPINA::SegmentationContourSwitch::~SegmentationContourSwitch()
{
}

//----------------------------------------------------------------------------
void ESPINA::SegmentationContourSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto opacity = settings->value(SEGMENTATION_CONTOUR_OPACITY_KEY, 0.6).toDouble();
  auto width   = settings->value(SEGMENTATION_CONTOUR_WIDTH_KEY, 0).toInt();
  auto pattern = settings->value(SEGMENTATION_CONTOUR_PATTERN_KEY, 0).toInt();

  m_settings->setOpacity(opacity);
  m_settings->setWidth(SegmentationContourPipeline::toWidth(width));
  m_settings->setPattern(SegmentationContourPipeline::toPattern(pattern));
}

//----------------------------------------------------------------------------
void ESPINA::SegmentationContourSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(SEGMENTATION_CONTOUR_OPACITY_KEY, m_settings->opacity());
  settings->setValue(SEGMENTATION_CONTOUR_WIDTH_KEY, SegmentationContourPipeline::widthValue(m_settings->width()));
  settings->setValue(SEGMENTATION_CONTOUR_PATTERN_KEY, SegmentationContourPipeline::patternValue(m_settings->pattern()));
}

//----------------------------------------------------------------------------
void SegmentationContourSwitch::onOpacityChanged()
{
  if (m_settings->opacity() == m_opacityWidget->value()/100.0) return;

  m_settings->setOpacity(m_opacityWidget->value()/100.0);
}

//----------------------------------------------------------------------------
void SegmentationContourSwitch::onWidthChanged()
{
  if(SegmentationContourPipeline::widthValue(m_settings->width()) == m_width->currentIndex()) return;

  m_settings->setWidth(SegmentationContourPipeline::toWidth(m_width->currentIndex()));
}

//----------------------------------------------------------------------------
void SegmentationContourSwitch::onPatternChanged()
{
  if(SegmentationContourPipeline::patternValue(m_settings->pattern()) == m_pattern->currentIndex()) return;

  m_settings->setPattern(SegmentationContourPipeline::toPattern(m_pattern->currentIndex()));
}

//----------------------------------------------------------------------------
void SegmentationContourSwitch::onSettingsModified()
{
  m_opacityWidget->setValue(m_settings->opacity()*100);
  m_width->setCurrentIndex(SegmentationContourPipeline::widthValue(m_settings->width()));
  m_pattern->setCurrentIndex(SegmentationContourPipeline::patternValue(m_settings->pattern()));

  auto items = m_manager->pools().first()->sources();

  invalidateRepresentations(items);
}

//----------------------------------------------------------------------------
void SegmentationContourSwitch::initWidgets()
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

  auto widthLabel = new QLabel("Width");
  addSettingsWidget(widthLabel);

  m_width = new QComboBox();
  m_width->insertItems(0, QStringList{ "Tiny", "Small", "Medium", "Large", "Big" });
  m_width->setCurrentIndex(0);

  connect(m_width, SIGNAL(currentIndexChanged(int)),
          this,    SLOT(onWidthChanged()));

  addSettingsWidget(m_width);

  auto patternLabel = new QLabel("Pattern");
  addSettingsWidget(patternLabel);

  m_pattern = new QComboBox();
  m_pattern->insertItems(0, QStringList{ "Normal", "Dotted", "Dashed" });
  m_pattern->setCurrentIndex(0);

  connect(m_pattern, SIGNAL(currentIndexChanged(int)),
          this,    SLOT(onPatternChanged()));

  addSettingsWidget(m_pattern);
}
