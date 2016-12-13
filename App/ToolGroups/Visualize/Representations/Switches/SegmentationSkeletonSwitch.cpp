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

using namespace ESPINA;
using namespace ESPINA::Representations;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Widgets::Styles;

const QString SEGMENTATION_SKELETON_OPACITY_KEY = "Opacity";
const QString SEGMENTATION_SKELETON_WIDTH_KEY   = "Width";
const QString SEGMENTATION_SKELETON_SHOWIDS_KEY = "ShowIds";

//---------------------------------------------------------------------
SegmentationSkeletonSwitch::SegmentationSkeletonSwitch(const QString &id,
                                                       RepresentationManagerSPtr manager,
                                                       std::shared_ptr<SegmentationSkeletonPoolSettings> settings,
                                                       ViewTypeFlags supportedViews,
                                                       Support::Context& context)
: BasicRepresentationSwitch(id, manager, supportedViews, context)
, m_settings               {settings}
, m_opacityWidget          {nullptr}
, m_widthWidget            {nullptr}
, m_annotationsWidget      {nullptr}
{
  initWidgets();

  connect(settings.get(), SIGNAL(modified()),
          this,           SLOT(onSettingsModified()));
}

//---------------------------------------------------------------------
SegmentationSkeletonSwitch::~SegmentationSkeletonSwitch()
{
  // created widgets destroyed by Qt
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  auto opacity = settings->value(SEGMENTATION_SKELETON_OPACITY_KEY, 0.6).toDouble();
  auto width   = settings->value(SEGMENTATION_SKELETON_WIDTH_KEY,     2).toInt();
  auto show    = settings->value(SEGMENTATION_SKELETON_SHOWIDS_KEY, false).toBool();

  m_settings->setOpacity(opacity);
  m_settings->setWidth(width);
  m_settings->setShowAnnotations(show);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(SEGMENTATION_SKELETON_OPACITY_KEY, m_settings->opacity());
  settings->setValue(SEGMENTATION_SKELETON_WIDTH_KEY,   m_settings->width());
  settings->setValue(SEGMENTATION_SKELETON_SHOWIDS_KEY, m_settings->showAnnotations());
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onOpacityChanged()
{
  if (m_settings->opacity() == m_opacityWidget->value()/100.0) return;

  m_settings->setOpacity(m_opacityWidget->value()/100.0);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onWidthChanged()
{
  if(m_settings->width() == m_widthWidget->currentIndex()) return;

  m_settings->setWidth(m_widthWidget->currentIndex());
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onAnnotationsVisibilityChanged()
{
  if(m_settings->showAnnotations() == m_annotationsWidget->isChecked()) return;

  m_settings->setShowAnnotations(m_annotationsWidget->isChecked());
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::onSettingsModified()
{
  m_opacityWidget->setValue(m_settings->opacity()*100);
  m_widthWidget->setCurrentIndex(m_settings->width());
  m_annotationsWidget->setChecked(m_settings->showAnnotations());

  auto items = m_manager->pools().first()->sources();

  invalidateRepresentations(items);
}

//---------------------------------------------------------------------
void SegmentationSkeletonSwitch::initWidgets()
{
  m_opacityWidget = new GUI::Widgets::NumericalInput();
  m_opacityWidget->setLabelText(tr("Opacity"));
  m_opacityWidget->setMinimum(0);
  m_opacityWidget->setMaximum(100);
  m_opacityWidget->setSliderTracking(false);
  m_opacityWidget->setValue(m_settings->opacity()*100);
  m_opacityWidget->setSpinBoxVisibility(false);
  m_opacityWidget->setWidgetsToolTip(tr("Skeleton representation opacity."));

  addSettingsWidget(m_opacityWidget);

  connect(m_opacityWidget, SIGNAL(valueChanged(int)),
          this,            SLOT(onOpacityChanged()));

  auto widthLabel = new QLabel("Width");
  widthLabel->setToolTip(tr("Skeleton representation line width"));

  addSettingsWidget(widthLabel);

  m_widthWidget = new QComboBox();
  m_widthWidget->insertItems(0, QStringList{ "Tiny", "Small", "Medium", "Large", "Big" });
  m_widthWidget->setCurrentIndex(m_settings->width());
  m_widthWidget->setToolTip(tr("Skeleton representation line width"));

  connect(m_widthWidget, SIGNAL(currentIndexChanged(int)),
          this,          SLOT(onWidthChanged()));

  addSettingsWidget(m_widthWidget);

  m_annotationsWidget = createToolButton(":/espina/tubular_id.svg", tr("Show node annotations"), nullptr);
  m_annotationsWidget->setCheckable(true);
  m_annotationsWidget->setChecked(m_settings->showAnnotations());

  connect(m_annotationsWidget, SIGNAL(toggled(bool)),
          this,                    SLOT(onAnnotationsVisibilityChanged()));

  addSettingsWidget(m_annotationsWidget);
}
