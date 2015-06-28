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

#include "SeedGrowSegmentationRefineWidget.h"
#include "ui_SeedGrowSegmentationRefineWidget.h"

#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <Settings/ROI/ROISettings.h>
#include <GUI/Dialogs/DefaultDialogs.h>

#include <QMessageBox>
#include <QUndoStack>
#include <QToolBar>

using namespace ESPINA;

class DiscardROIModificationsCommand
: public QUndoCommand
{
public:
  explicit DiscardROIModificationsCommand(RestrictToolGroup *roiTools, SeedGrowSegmentationFilterSPtr filter, QUndoCommand* parent = 0)
  : m_roiTools{roiTools}
  {
    if (filter->roi())
    {
      m_ROI = filter->roi()->clone();
    }
  }

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
  RestrictToolGroup *m_roiTools;
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
  SGSFilterModification(SegmentationAdapterPtr         segmentation,
                        SeedGrowSegmentationFilterSPtr filter,
                        ROISPtr                        roi,
                        int                            threshold,
                        int                            closeRadius,
                        QUndoCommand                  *parent = nullptr)
  : QUndoCommand(parent)
  , m_segmentation(segmentation)
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
//     int volumeSize = 1;
//     for (auto dir : {Axis::X, Axis::Y, Axis::Z})
//     {
//       volumeSize *= volume->bounds().lenght(dir);
//     }

    // if (!m_oldVolume && (output->isEdited() || volumeSize < MAX_UNDO_SIZE))
    if (!m_oldVolume && output->isEdited())
    {
      auto volume = readLockVolume(output);
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

    invalidateRepresentations();
  }

  virtual void undo()
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
    } else
    {
      update();
    }

    invalidateRepresentations();
  }

private:
  void update()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_filter->update();
    QApplication::restoreOverrideCursor();
  }

  void invalidateRepresentations()
  {
    m_segmentation->invalidateRepresentations();
  }

private:
  SegmentationAdapterPtr         m_segmentation;
  SeedGrowSegmentationFilterSPtr m_filter;

  ROISPtr m_ROI,           m_oldROI;
  int     m_threshold,     m_oldThreshold;
  int     m_closingRadius, m_oldClosingRadius;

  Bounds                 m_oldBounds;
  itkVolumeType::Pointer m_oldVolume;
  BoundsList             m_editedRegions;
  //itkVolumeType::Pointer m_newVolume;
};

//----------------------------------------------------------------------------
SeedGrowSegmentationRefineWidget::SeedGrowSegmentationRefineWidget(SegmentationAdapterPtr         segmentation,
                                                                     SeedGrowSegmentationFilterSPtr filter,
                                                                     RestrictToolGroup             *roiTools,
                                                                     Support::Context              &context)
: WithContext(context)
, m_segmentation(segmentation)
, m_gui(new Ui::SeedGrowSegmentationRefineWidget())
, m_filter(filter)
, m_roiTools(roiTools)
{
  m_gui->setupUi(this);

  auto toolbar = new QToolBar();

  // TODO: create aux function to populate toolbar with tool group actions
  for (auto tools : m_roiTools->groupedTools())
  {
    for (auto tool : tools)
    {
      for (auto action : tool->actions())
      {
        toolbar->addAction(action);
      }
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
}

//----------------------------------------------------------------------------
SeedGrowSegmentationRefineWidget::~SeedGrowSegmentationRefineWidget()
{
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onThresholdChanged(int value)
{
  m_gui->apply->setEnabled(true);

  emit thresholdChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onApplyClosingChanged(bool value)
{
  m_gui->apply->setEnabled(true);

  m_gui->closingRadius->setEnabled(value);
  m_gui->closingRadiusLabel->setEnabled(value);

  emit applyClosingChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onClosingRadiusChanged(int value)
{
  emit closingRadiusChanged(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onROIChanged()
{
  m_roiTools->setEnabled(true);
}


//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::onDiscardROIModifications()
{
  auto undoStack = getUndoStack();
  undoStack->beginMacro(tr("Discard ROI modifications"));
  undoStack->push(new DiscardROIModificationsCommand(m_roiTools, m_filter));
  undoStack->endMacro();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::modifyFilter()
{
  auto output = m_filter->output(0);

  if (!output->isEdited() || discardChangesConfirmed())
  {
    auto volume = readLockVolume(output);

    auto spacing = volume->spacing();
    auto roi     = m_roiTools->currentROI();
    auto seed    = m_filter->seed();

    if (!roi  || contains(roi.get(), seed, spacing))
    {
      auto undoStack = getUndoStack();
      auto threshold = m_gui->threshold->value();
      auto radius    = m_gui->closingRadius->value();

      undoStack->beginMacro("Modify Seed Grow Segmentation Parameters");
      undoStack->push(new SGSFilterModification(m_segmentation, m_filter, roi, threshold, radius));
      undoStack->endMacro();

      if (m_filter->isTouchingROI())
      {
        auto message = tr("New segmentation may be incomplete due to ROI restriction.");

        GUI::DefaultDialogs::InformationMessage(dialogTitle(), message);
      }

      auto currentFilterROI = m_filter->roi();
      if (currentFilterROI)
      {
        m_roiTools->setCurrentROI(currentFilterROI->clone());
      }
    }
    else
    {
      auto message = tr("Segmentation couldn't be modified. Seed is outside ROI");

      GUI::DefaultDialogs::InformationMessage(dialogTitle(), message);
    }
  }
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::setThreshold(int value)
{
  m_gui->threshold->setValue(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::setApplyClosing(bool value)
{
  m_gui->applyClosing->setChecked(value);
  m_gui->closingRadius->setEnabled(value);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationRefineWidget::setClosingRadius(int value)
{
  m_gui->closingRadius->setValue(value);
}

//----------------------------------------------------------------------------
QString SeedGrowSegmentationRefineWidget::dialogTitle() const
{
  return tr("Seed Grow Segmentation");
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationRefineWidget::discardChangesConfirmed() const
{
  auto message = tr("Filter contains segmentations that have been manually modified by the user."
                    "Updating this filter will result in losing user modifications."
                    "Do you want to proceed?");

  return GUI::DefaultDialogs::UserConfirmation(dialogTitle() ,message);
}
