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

#include "SeedGrowSegmentationHistoryWidget.h"
#include "ui_SeedGrowSegmentationHistoryWidget.h"

#include <ToolGroups/ROI/ROITools.h>
#include <Settings/ROI/ROISettings.h>

#include <QMessageBox>
#include <QUndoStack>
#include <QToolBar>

using namespace ESPINA;

class DiscardROIModificationsCommand
: public QUndoCommand
{
public:
  explicit DiscardROIModificationsCommand(ROIToolsGroup *roiTools, SeedGrowSegmentationFilterSPtr filter, QUndoCommand* parent = 0)
  : m_roiTools{roiTools}
  , m_ROI{filter->roi()->clone()}
  {}

  virtual void redo()
  { swapCurrentROI(); }

  virtual void undo()
  { swapCurrentROI(); }

private:
  void swapCurrentROI()
  {
    auto currentROI = m_roiTools->currentROI();

    m_roiTools->setCurrentROI(m_ROI);

    m_ROI = currentROI;
  }

private:
  ROIToolsGroup *m_roiTools;
  ROISPtr        m_ROI;
};

class SGSFilterModification
: public QUndoCommand
{
public:
  /** \brief Create a new undo command to modify a SeedGrowSegmentationFilter
   *
   *  \param[in] filter the SeedGrowSegmentationFilter to modify
   *  \param[in] roi to be applied to limit the seed grow algorithm
   *  \param[in] threshold symmetric threshold to be used to determine connectivity respect
   *                       from the gray level value of the seed voxel
   *  \param[in] closeRadius value of the radius to be applied to the closing post-proccessing.
   *                       If this value is 0 no post-proccessing will be executed
   *  \param[in] parent the undo command which will trigger this one
   */
  SGSFilterModification(SeedGrowSegmentationFilterSPtr filter,
                        ROISPtr                        roi,
                        int                            threshold,
                        int                            closeRadius,
                        QUndoCommand                  *parent = nullptr)
  : QUndoCommand(parent)
  , m_filter(filter)
  , m_ROI(roi)
  , m_threshold(threshold)
  , m_closingRadius(closeRadius)
  {
    m_oldROI           = m_filter->roi();
    m_oldThreshold     = m_filter->lowerThreshold();
    m_oldClosingRadius = m_filter->closingRadius();
  }

  virtual void redo()
  {
    auto output = m_filter->output(0);
    auto volume = volumetricData(output);
//     int volumeSize = 1;
//     for (auto dir : {Axis::X, Axis::Y, Axis::Z})
//     {
//       volumeSize *= volume->bounds().lenght(dir);
//     }

    // if (!m_oldVolume && (output->isEdited() || volumeSize < MAX_UNDO_SIZE))
    if (!m_oldVolume && output->isEdited())
    {
      m_oldBounds     = volume->bounds();
      m_oldVolume     = volume->itkImage();
      m_editedRegions = volume->editedRegions();
    }

    //bool ignoreUpdate = m_newVolume.IsNotNull();

    m_filter->setLowerThreshold(m_threshold);
    m_filter->setUpperThreshold(m_threshold);
    m_filter->setROI(m_ROI);
    m_filter->setClosingRadius(m_closingRadius);

    update();
  }

  virtual void undo()
  {
    m_filter->setLowerThreshold(m_oldThreshold);
    m_filter->setUpperThreshold(m_oldThreshold);
    m_filter->setROI(m_oldROI);
    m_filter->setClosingRadius(m_oldClosingRadius);

    auto output = m_filter->output(0);
    auto volume = volumetricData(output);

    if (m_oldVolume.IsNotNull())
    {
      volume->resize(m_oldBounds);
      volume->draw(m_oldVolume);
      volume->setEditedRegions(m_editedRegions);
    } else
    {
      update();
    }
  }

private:
  void update()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_filter->update();
    QApplication::restoreOverrideCursor();
  }

private:
  SeedGrowSegmentationFilterSPtr m_filter;

  ROISPtr m_ROI,           m_oldROI;
  int     m_threshold,     m_oldThreshold;
  int     m_closingRadius, m_oldClosingRadius;

  Bounds m_oldBounds;
  itkVolumeType::Pointer m_oldVolume;
  BoundsList             m_editedRegions;
  //itkVolumeType::Pointer m_newVolume;
};

//----------------------------------------------------------------------------
SeedGrowSegmentationHistoryWidget::SeedGrowSegmentationHistoryWidget(SeedGrowSegmentationFilterSPtr filter,
                                                                     ROIToolsGroup                 *roiTools,
                                                                     ViewManagerSPtr                viewManager,
                                                                     QUndoStack                    *undoStack,
                                                                     QWidget                       *parent,
                                                                     Qt::WindowFlags                flags)
: QWidget(parent, flags)
, m_gui(new Ui::SeedGrowSegmentationHistoryWidget())
, m_filter(filter)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
, m_roiTools(roiTools)
{
  m_gui->setupUi(this);

  auto toolbar = new QToolBar();

  for (auto tool : m_roiTools->tools())
  {
    for (auto action : tool->actions())
    {
      toolbar->addAction(action);
    }
  }

  m_gui->roiFrame->layout()->addWidget(toolbar);

  auto seed = m_filter->seed();
  m_gui->seed->setText(QString("(%1, %2, %3)").arg(seed[0]).arg(seed[1]).arg(seed[2]));
  m_gui->threshold->setMaximum(255);
  m_gui->threshold->setValue(m_filter->lowerThreshold());
  m_gui->closingRadius->setValue(m_filter->closingRadius());

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
  connect(m_roiTools,                     SIGNAL(roiChanged(ROISPtr)),
          this,                           SLOT(onROIChanged()));

  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
SeedGrowSegmentationHistoryWidget::~SeedGrowSegmentationHistoryWidget()
{
  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::onThresholdChanged(int value)
{
  m_gui->apply->setEnabled(true);

  emit thresholdChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::onApplyClosingChanged(bool value)
{
  m_gui->apply->setEnabled(true);

  m_gui->closingRadius->setEnabled(value);
  m_gui->closingRadiusLabel->setEnabled(value);

  emit applyClosingChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::onClosingRadiusChanged(int value)
{
  emit closingRadiusChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::onROIChanged()
{
  m_roiTools->setEnabled(true);
}


//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::onDiscardROIModifications()
{
  m_undoStack->beginMacro(tr("Discard ROI modifications"));
  m_undoStack->push(new DiscardROIModificationsCommand(m_roiTools, m_filter));
  m_undoStack->endMacro();
  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::modifyFilter()
{
  auto output = m_filter->output(0);

  if (output->isEdited())
  {
    QMessageBox msg;
    msg.setText(tr("Filter contains segmentations that have been manually modified by the user."
                   "Updating this filter will result in losing user modifications."
                   "Do you want to proceed?"));
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);

    if (msg.exec() != QMessageBox::Yes)
      return;
  }

  auto volume = volumetricData(output);

  auto spacing = volume->spacing();
  auto roi     = m_roiTools->currentROI();

  if (roi && !contains(roi, m_filter->seed(), spacing))
  {
    QMessageBox::warning(this,
                         tr("Seed Grow Segmentation"),
                         tr("Segmentation couldn't be modified. Seed is outside ROI"));
                         return;
  }

  m_undoStack->beginMacro("Modify Seed Grow Segmentation Parameters");
  {
    m_undoStack->push(new SGSFilterModification(m_filter, roi, m_gui->threshold->value(), m_gui->closingRadius->value()));
  }
  m_undoStack->endMacro();

  if (m_filter->isTouchingROI())
  {
    QMessageBox warning;
    warning.setIcon(QMessageBox::Warning);
    warning.setWindowTitle(tr("Seed Grow Segmentation Filter Information"));
    warning.setText(tr("New segmentation may be incomplete due to ROI restriction."));
    warning.exec();
  }

  if (m_filter->roi())
  {
    m_roiTools->setCurrentROI(m_filter->roi()->clone());
  }

  m_viewManager->updateSegmentationRepresentations();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::setThreshold(int value)
{
  m_gui->threshold->setValue(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::setApplyClosing(bool value)
{
  m_gui->applyClosing->setChecked(value);
  m_gui->closingRadius->setEnabled(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationHistoryWidget::setClosingRadius(int value)
{
  m_gui->closingRadius->setValue(value);
}
