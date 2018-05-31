/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

// ESPINA
#include "SeedGrowSegmentationRefineWidget.h"
#include "SeedTemporalPrototype.h"
#include "ui_SeedGrowSegmentationRefineWidget.h"
#include <Core/Utils/SignalBlocker.h>
#include <App/ToolGroups/Restrict/RestrictToolGroup.h>
#include <App/Settings/ROI/ROISettings.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>

// Qt
#include <QMessageBox>
#include <QUndoStack>
#include <QToolBar>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Representations::Managers;

// BEGIN DEBUG Only
bool SeedGrowSegmentationRefineWidget::s_exists = false;
QMutex SeedGrowSegmentationRefineWidget::s_mutex;
// END DEBUG Only

// TODO: 26-11-2015 @felix - Would be interesting to add "TouchesROI" to the data shown on the widget,
// but requires recalculation after loading from disk or storing the value on the SEG with the rest of
// the filter data.

//----------------------------------------------------------------------------
DiscardROIModificationsCommand::DiscardROIModificationsCommand(RestrictToolGroupSPtr roiTools, SeedGrowSegmentationFilterSPtr filter, QUndoCommand* parent)
: m_roiTools{roiTools}
{
  if (filter->roi())
  {
    m_ROI = filter->roi()->clone();
  }
}

//----------------------------------------------------------------------------
void DiscardROIModificationsCommand::redo()
{
  swapCurrentROI();
}

//----------------------------------------------------------------------------
void DiscardROIModificationsCommand::undo()
{
  swapCurrentROI();
}

//----------------------------------------------------------------------------
void DiscardROIModificationsCommand::swapCurrentROI()
{
  auto currentROI = m_roiTools->currentROI();

  m_roiTools->setCurrentROI(m_ROI);

  m_ROI = currentROI;
}

//----------------------------------------------------------------------------
SGSFilterModification::SGSFilterModification(SegmentationAdapterPtr         segmentation,
                                             ROISPtr                        roi,
                                             int                            threshold,
                                             int                            closeRadius,
                                             QUndoCommand                  *parent)
: QUndoCommand   {parent}
, m_segmentation {segmentation}
, m_filter       {std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(segmentation->filter())}
, m_ROI          {roi}
, m_threshold    {threshold}
, m_closingRadius{closeRadius}
{
  m_oldROI           = m_filter->roi();
  m_oldThreshold     = m_filter->lowerThreshold();
  m_oldClosingRadius = m_filter->closingRadius();
}

//----------------------------------------------------------------------------
void SGSFilterModification::redo()
{
  auto output = m_filter->output(0);

  if (!m_oldVolume && output->isEdited())
  {
    auto volume = readLockVolume(output);
    m_oldBounds = volume->bounds();
    m_oldVolume = volume->itkImage();
    m_editedRegions = volume->editedRegions();
  }

  m_filter->setLowerThreshold(m_threshold);
  m_filter->setUpperThreshold(m_threshold);
  m_filter->setROI(m_ROI);
  m_filter->setClosingRadius(m_closingRadius);

  update();

  invalidateRepresentations();
}

//----------------------------------------------------------------------------
void SGSFilterModification::undo()
{
  m_filter->setLowerThreshold(m_oldThreshold);
  m_filter->setUpperThreshold(m_oldThreshold);
  m_filter->setROI(m_oldROI);
  m_filter->setClosingRadius(m_oldClosingRadius);

  auto output = m_filter->output(0);

  if (m_oldVolume.IsNotNull())
  {
    auto volume = writeLockVolume(output);
    volume->resize(m_oldBounds);
    volume->draw(m_oldVolume);
    volume->setEditedRegions(m_editedRegions);
  }
  else
  {
    update();
  }

  invalidateRepresentations();
}

//----------------------------------------------------------------------------
void SGSFilterModification::update()
{
  Styles::WaitingCursor cursor;
  m_filter->update();
}

//----------------------------------------------------------------------------
void SGSFilterModification::invalidateRepresentations()
{
  m_segmentation->invalidateRepresentations();
}

//----------------------------------------------------------------------------
SeedGrowSegmentationRefineWidget::SeedGrowSegmentationRefineWidget(SegmentationAdapterPtr segmentation,
                                                                   Support::Context      &context,
                                                                   QWidget               *parent)
: QWidget       {parent}
, WithContext   (context)
, m_segmentation{segmentation}
, m_gui         {new Ui::SeedGrowSegmentationRefineWidget()}
, m_filter      {std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(segmentation->filter())}
{
  s_mutex.lock();
  Q_ASSERT(!s_exists);
  s_exists = true;
  s_mutex.unlock();

  m_gui->setupUi(this);

  auto toolbar = new QToolBar(m_gui->roiFrame);
  toolbar->setMinimumHeight(Styles::CONTEXTUAL_BAR_HEIGHT);

  m_roiTools = std::make_shared<RestrictToolGroup>(new ROISettings(), context, toolbar);

  populateToolBar(toolbar, m_roiTools->groupedTools());

  QColor sgsROIColor{Qt::yellow};
  sgsROIColor.setHslF(sgsROIColor.hueF(),sgsROIColor.saturationF(), 0.9);
  m_roiTools->setColor(sgsROIColor);

  m_gui->roiFrame->layout()->addWidget(toolbar);

  auto seed = m_filter->seed();

  auto text = tr("<a href=""%1"">%1</a>").arg(seed.toString());
  m_gui->seed->setText(text);
  m_gui->seed->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  m_gui->seed->setOpenExternalLinks(false);
  m_gui->seed->setTextFormat(Qt::RichText);
  connect(m_gui->seed, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));

  m_gui->threshold->setMaximum(255);
  m_gui->threshold->setValue(m_filter->lowerThreshold());
  m_gui->closingRadius->setValue(m_filter->closingRadius());

  auto enabled = (m_filter->closingRadius() != 0);
  m_gui->applyClosing->setChecked(enabled);
  m_gui->closingRadius->setEnabled(enabled);
  m_gui->closingRadiusLabel->setEnabled(enabled);

  m_gui->apply->setEnabled(false);

  auto roi = m_filter->roi();
  if(roi)
  {
    m_roiTools->setCurrentROI(roi->clone());
    m_roiTools->setVisible(true);
  }

  auto prototype2D = std::make_shared<SeedTemporalRepresentation>(m_filter.get());
  m_seedPrototypes = std::make_shared<TemporalPrototypes>(prototype2D, TemporalRepresentation3DSPtr(), tr("SeedTempRep"));
  context.viewState().addTemporalRepresentations(m_seedPrototypes);

  connect(m_gui->threshold,               SIGNAL(valueChanged(int)),
          this,                           SLOT(onThresholdChanged(int)));
  connect(m_gui->applyClosing,            SIGNAL(toggled(bool)),
          this,                           SLOT(onApplyClosingChanged(bool)));
  connect(m_gui->closingRadius,           SIGNAL(valueChanged(int)),
          this,                           SLOT(onClosingRadiusChanged(int)));
  connect(m_gui->discardROIModifications, SIGNAL(clicked(bool)),
          this,                           SLOT(onDiscardROIModifications()));
  connect(m_gui->apply,                   SIGNAL(clicked(bool)),
          this,                           SLOT(modifyFilter()));
  connect(m_roiTools.get(),               SIGNAL(ROIChanged(ROISPtr)),
          this,                           SLOT(onROIChanged()));

  connect(m_filter.get(), SIGNAL(thresholdModified(int, int)),
          this,           SLOT(onFilterThresholdModified(int, int)));
  connect(m_filter.get(), SIGNAL(radiusModified(int)),
          this,           SLOT(onFilterRadiusModified(int)));
  connect(m_filter.get(), SIGNAL(roiModified(ROISPtr)),
          this,           SLOT(onFilterroiModified(ROISPtr)));
}

//----------------------------------------------------------------------------
SeedGrowSegmentationRefineWidget::~SeedGrowSegmentationRefineWidget()
{
  if(m_gui->apply->isEnabled())
  {
    auto answer = GUI::DefaultDialogs::UserQuestion(tr("The properties of the segmentation '%1' have been modified but haven't been applied, do you want to discard them?").arg(m_segmentation->data().toString()),
                                                    QMessageBox::Apply|QMessageBox::Discard,
                                                    dialogTitle());

    if(answer == QMessageBox::Apply)
    {
      modifyFilter();
    }
  }

  s_mutex.lock();
  Q_ASSERT(s_exists);
  s_exists = false;
  s_mutex.unlock();

  getContext().viewState().removeTemporalRepresentations(m_seedPrototypes);

  if(m_roiTools->currentROI() != nullptr)
  {
    m_roiTools->setCurrentROI(nullptr);
    m_roiTools->setVisible(false);
  }
}
//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onFilterThresholdModified(int lower, int upper)
{
  // NOTE: assuming symmetric threshold.
  m_gui->threshold->setValue(lower);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onFilterRadiusModified(int value)
{
  auto enabled = (value != 0);

  m_gui->closingRadius->setValue(value);

  m_gui->applyClosing->setChecked(enabled);
  m_gui->closingRadius->setEnabled(enabled);
  m_gui->closingRadiusLabel->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onFilterroiModified(ROISPtr roi)
{
  m_roiTools->setCurrentROI(roi);
  m_roiTools->setVisible(roi != nullptr);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onThresholdChanged(int value)
{
  auto enabled = (m_filter->upperThreshold() != value);

  m_gui->apply->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onApplyClosingChanged(bool value)
{
  auto filterClosingEnabled = (m_filter->closingRadius() != 0);
  auto enabled = (value != filterClosingEnabled);

  m_gui->apply->setEnabled(enabled);

  m_gui->closingRadius->setEnabled(value);
  m_gui->closingRadiusLabel->setEnabled(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onClosingRadiusChanged(int value)
{
  auto enabled = (value != m_filter->closingRadius());

  m_gui->apply->setEnabled(enabled);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onROIChanged()
{
  m_roiTools->setEnabled(true);
  m_gui->apply->setEnabled(true);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onDiscardROIModifications()
{
  auto undoStack = getUndoStack();
  undoStack->beginMacro(tr("Discard ROI modifications of '%1'.").arg(m_segmentation->data().toString()));
  undoStack->push(new DiscardROIModificationsCommand(m_roiTools, m_filter));
  undoStack->endMacro();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::modifyFilter()
{
  auto output = m_filter->output(0);

  SignalBlocker<OutputSPtr> blocker(output);

  if (output->isEdited())
  {
    auto buttons = QMessageBox::Yes|QMessageBox::Cancel;
    auto message = tr("Filter contains segmentations that have been manually modified by the user."
                  "Updating this filter will result in losing user modifications."
                  "Do you want to proceed?");

    if (GUI::DefaultDialogs::UserQuestion(message, buttons, dialogTitle()) == QMessageBox::Cancel) return;
  }

  auto volume = readLockVolume(output);

  auto spacing = volume->bounds().spacing();
  auto roi = m_roiTools->currentROI();
  auto seed = m_filter->seed();

  if (!roi || contains(roi.get(), seed, spacing))
  {
    auto undoStack = getUndoStack();
    auto threshold = m_gui->threshold->value();
    auto radius = m_gui->closingRadius->value();

    undoStack->beginMacro(tr("Modify grey level segmentation parameters of '%1'.").arg(m_segmentation->data().toString()));
    undoStack->push(new SGSFilterModification(m_segmentation, roi, threshold, radius));
    undoStack->endMacro();

    if (m_filter->isTouchingROI())
    {
      auto message = tr("New segmentation may be incomplete due to ROI restrictions.");

      GUI::DefaultDialogs::InformationMessage(message, dialogTitle());
    }

    auto currentFilterROI = m_filter->roi();
    if (currentFilterROI)
    {
      m_roiTools->setCurrentROI(currentFilterROI->clone());
    }

    m_gui->apply->setEnabled(false);
  }
  else
  {
    auto message = tr("Segmentation couldn't be modified because the ROI doesn't contain the <b>seed point</b>.");

    GUI::DefaultDialogs::InformationMessage(message, dialogTitle());
  }
}

//----------------------------------------------------------------------------
QString SeedGrowSegmentationRefineWidget::dialogTitle() const
{
  return tr("Grey Level Segmentation");
}

//--------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onLinkActivated(const QString &link)
{
  NmVector3 point{link};

  getViewState().focusViewOn(point);
}
