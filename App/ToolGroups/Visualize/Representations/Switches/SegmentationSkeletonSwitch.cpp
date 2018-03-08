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

// ESPINA
#include <ToolGroups/Visualize/Representations/Switches/SegmentationSkeletonSwitch.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Widgets/Styles.h>

// Qt
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QSlider>

using namespace ESPINA;
using namespace ESPINA::Representations;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::Widgets::Styles;

const QString SEGMENTATION_SKELETON_WIDTH_KEY   = "Width";
const QString SEGMENTATION_SKELETON_SHOWIDS_KEY = "ShowIds";
const QString SEGMENTATION_SKELETON_IDS_SIZE    = "IdsSize";

//---------------------------------------------------------------------
SegmentationSkeletonSwitch::SegmentationSkeletonSwitch(const QString &id,
                                                       RepresentationManagerSPtr manager,
                                                       SkeletonPoolSettingsSPtr settings,
                                                       ViewTypeFlags supportedViews,
                                                       Support::Context& context)
: BasicRepresentationSwitch{id, manager, supportedViews, context}
, m_settings               {settings}
{
  initWidgets();

  connect(settings.get(), SIGNAL(modified()),
          this,           SLOT(onSettingsModified()));
}

//---------------------------------------------------------------------
SegmentationSkeletonSwitch::~SegmentationSkeletonSwitch()
{
  disconnect(m_settings.get(), SIGNAL(modified()),
             this,             SLOT(onSettingsModified()));
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto width   = settings->value(SEGMENTATION_SKELETON_WIDTH_KEY,       2).toInt();
  auto show    = settings->value(SEGMENTATION_SKELETON_SHOWIDS_KEY, false).toBool();
  auto size    = settings->value(SEGMENTATION_SKELETON_IDS_SIZE,       15).toInt();

  m_settings->setWidth(std::max(1, std::min(width, 5)));
  m_settings->setShowAnnotations(show);
  m_settings->setAnnotationsSize(size);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(SEGMENTATION_SKELETON_WIDTH_KEY,   m_settings->width());
  settings->setValue(SEGMENTATION_SKELETON_SHOWIDS_KEY, m_settings->showAnnotations());
  settings->setValue(SEGMENTATION_SKELETON_IDS_SIZE,    m_settings->annotationsSize());
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onWidthChanged()
{
  if(m_settings->width() == m_widthWidget->currentIndex() + 1) return;

  m_settings->setWidth(m_widthWidget->currentIndex() + 1);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onAnnotationsVisibilityChanged()
{
  auto enabled = m_annotationsWidget->isChecked();
  if(m_settings->showAnnotations() == enabled) return;

  m_settings->setShowAnnotations(enabled);
  m_annotationsTextWidget->setVisible(enabled);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onAnnotationsSizeChanged(int value)
{
  if(m_settings->annotationsSize() == value) return;

  m_settings->setAnnotationsSize(value);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onSettingsModified()
{
  m_widthWidget->setCurrentIndex(m_settings->width() - 1);
  m_annotationsWidget->setChecked(m_settings->showAnnotations());
  m_annotationsTextWidget->setValue(m_settings->annotationsSize());
  m_annotationsTextWidget->setVisible(m_settings->showAnnotations());

  auto items = m_manager->pools().first()->sources();

  invalidateRepresentations(items);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::initWidgets()
{
  auto widthLabel = new QLabel("Width");
  widthLabel->setToolTip(tr("Skeleton representation line width"));

  addSettingsWidget(widthLabel);

  m_widthWidget = new QComboBox();
  m_widthWidget->insertItems(0, QStringList{ "Tiny", "Small", "Medium", "Large", "Big" });
  m_widthWidget->setCurrentIndex(m_settings->width() - 1);
  m_widthWidget->setToolTip(tr("Skeleton representation line width"));

  connect(m_widthWidget, SIGNAL(currentIndexChanged(int)),
          this,          SLOT(onWidthChanged()));

  addSettingsWidget(m_widthWidget);

  m_annotationsWidget = createToolButton(":/espina/tubular_id.svg", tr("Show stroke names"), nullptr);
  m_annotationsWidget->setCheckable(true);
  m_annotationsWidget->setChecked(m_settings->showAnnotations());

  connect(m_annotationsWidget, SIGNAL(toggled(bool)),
          this,                SLOT(onAnnotationsVisibilityChanged()));

  addSettingsWidget(m_annotationsWidget);

  m_annotationsTextWidget = new GUI::Widgets::NumericalInput();
  m_annotationsTextWidget->setMinimum(5);
  m_annotationsTextWidget->setMaximum(15);
  m_annotationsTextWidget->setValue(m_settings->annotationsSize());
  m_annotationsTextWidget->setLabelText("Size");
  m_annotationsTextWidget->setWidgetsToolTip("Annotations text size");
  m_annotationsTextWidget->setLabelVisibility(true);
  m_annotationsTextWidget->setSliderVisibility(true);
  m_annotationsTextWidget->setSpinBoxVisibility(false);

  connect(m_annotationsTextWidget, SIGNAL(valueChanged(int)),
          this,                    SLOT(onAnnotationsSizeChanged(int)));

  addSettingsWidget(m_annotationsTextWidget);

  m_annotationsTextWidget->setVisible(m_settings->showAnnotations());
}

