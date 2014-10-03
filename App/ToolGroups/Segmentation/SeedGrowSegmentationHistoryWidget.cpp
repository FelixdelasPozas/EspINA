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

#include <QMessageBox>
#include <QUndoStack>

using namespace ESPINA;


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

//     if (!m_oldVolume && (output->isEdited() || volume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE))
//     {
//       m_oldVolume     = volume->cloneVolume();
//       m_editedRegions = output->editedRegions();
//     }

    bool ignoreUpdate = m_newVolume.IsNotNull();

//     m_filter->setLowerThreshold(m_threshold, ignoreUpdate);
//     m_filter->setUpperThreshold(m_threshold, ignoreUpdate);
//     m_filter->setROI(m_ROI, ignoreUpdate);
//     m_filter->setCloseValue(m_closingRadius, ignoreUpdate);

    m_filter->setLowerThreshold(m_threshold);
    m_filter->setUpperThreshold(m_threshold);
    m_filter->setROI(m_ROI);
    m_filter->setClosingRadius(m_closingRadius);

    if (m_newVolume.IsNull())
    {
      update();

//       SegmentationVolumeSPtr newVolume = volume;
//       if (newVolume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE)
//         m_newVolume = volume->cloneVolume();
    }
    else
    {
//       volume->setVolume(m_newVolume);
    }

    output->clearEditedRegions();
  }

  virtual void undo()
  {
//     m_filter->setLowerThreshold(m_oldThreshold, true);
//     m_filter->setUpperThreshold(m_oldThreshold, true);
//     m_filter->setROI(m_oldROI, true);
//     m_filter->setCloseValue(m_oldClosingRadius, true);

    m_filter->setLowerThreshold(m_oldThreshold);
    m_filter->setUpperThreshold(m_oldThreshold);
    m_filter->setROI(m_oldROI);
    m_filter->setClosingRadius(m_oldClosingRadius);

//     SegmentationOutputSPtr output = boost::dynamic_pointer_cast<SegmentationOutput>(m_filter->output(0));
//     SegmentationVolumeSPtr volume = segmentationVolume(output);

    if (m_oldVolume.IsNotNull())
    {
//       volume->setVolume(m_oldVolume);
//       output->setEditedRegions(m_editedRegions);
    } else
    {
      update();
    }
  }

private:
  void update()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_filter->update(0);
    QApplication::restoreOverrideCursor();
  }

private:
  SeedGrowSegmentationFilterSPtr m_filter;

  ROISPtr m_ROI,           m_oldROI;
  int     m_threshold,     m_oldThreshold;
  int     m_closingRadius, m_oldClosingRadius;

  itkVolumeType::Pointer m_oldVolume;
  itkVolumeType::Pointer m_newVolume;

  //FilterOutput::EditedRegionSList m_editedRegions;
};

//----------------------------------------------------------------------------
SeedGrowSegmentationHistoryWidget::SeedGrowSegmentationHistoryWidget(std::shared_ptr<SeedGrowSegmentationFilter> filter,
                                                                     ViewManagerSPtr             viewManager,
                                                                     QUndoStack                 *undoStack,
                                                                     QWidget                                    *parent,
                                                                     Qt::WindowFlags                             f)
: QWidget(parent, f)
, m_gui(new Ui::SeedGrowSegmentationHistoryWidget())
, m_filter(filter)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
{
  m_gui->setupUi(this);

//   connect(filter, SIGNAL(modified(ModelItemPtr)),
//           this, SLOT(updateWidget()));

  //SegmentationVolumeSPtr volume = segmentationVolume(filter->output(0));

  auto seed = m_filter->seed();
  m_gui->m_xSeed->setText(QString("%1").arg(seed[0]));
  m_gui->m_ySeed->setText(QString("%1").arg(seed[1]));
  m_gui->m_zSeed->setText(QString("%1").arg(seed[2]));
  m_gui->m_threshold->setMaximum(255);
  m_gui->m_threshold->setValue(m_filter->lowerThreshold());

  m_gui->roiGroup->setVisible(false);
//   int voiExtent[6];
//   m_filter->voi(voiExtent);
//   itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
//   for (int i=0; i<6; i++)
//     m_voiBounds[i] = voiExtent[i] * spacing[i/2];
//
//   m_leftMargin->setValue(m_voiBounds[0]);
//   m_leftMargin->setSuffix(" nm");
//   m_leftMargin->installEventFilter(this);
//   connect(m_leftMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds()));
//
//   m_rightMargin->setValue(m_voiBounds[1]);
//   m_rightMargin->setSuffix(" nm");
//   m_rightMargin->installEventFilter(this);
//   connect(m_rightMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds()));
//
//   m_topMargin->setValue(m_voiBounds[2]);
//   m_topMargin->setSuffix(" nm");
//   m_topMargin->installEventFilter(this);
//   connect(m_topMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds()));
//
//   m_bottomMargin->setValue(m_voiBounds[3]);
//   m_bottomMargin->setSuffix(" nm");
//   m_bottomMargin->installEventFilter(this);
//   connect(m_bottomMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds( )));
//
//   m_upperMargin->setValue(m_voiBounds[4]);
//   m_upperMargin->setSuffix(" nm");
//   m_upperMargin->installEventFilter(this);
//   connect(m_upperMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds()));
//
//   m_lowerMargin->setValue(m_voiBounds[5]);
//   m_lowerMargin->setSuffix(" nm");
//   m_lowerMargin->installEventFilter(this);
//   connect(m_lowerMargin, SIGNAL(valueChanged(int)),
//           this, SLOT(updateRegionBounds()));
//
//   bool enabled = m_filter->closeValue() > 0;
//   m_closeCheckbox->setChecked(enabled);
//   connect(m_closeCheckbox, SIGNAL(stateChanged(int)),
//           this, SLOT(modifyCloseCheckbox(int)));
//
//   m_closeRadius->setEnabled(enabled);
//   m_closeRadius->setValue(m_filter->closeValue());
//   connect(m_closeRadius, SIGNAL(valueChanged(int)),
//           this, SLOT(modifyCloseValue(int)));
//
//
  connect(m_gui->m_modify, SIGNAL(clicked(bool)),
          this,            SLOT(modifyFilter()));
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

//   double voiBounds[6];
//   voiBounds[0] = m_leftMargin->value();
//   voiBounds[1] = m_rightMargin->value();
//   voiBounds[2] = m_topMargin->value();
//   voiBounds[3] = m_bottomMargin->value();
//   voiBounds[4] = m_upperMargin->value();
//   voiBounds[5] = m_lowerMargin->value();
//
//   int ROI[6];
//   for(int i=0; i < 6; i++)
//     ROI[i] = voiBounds[i]/spacing[i/2];
//
//   int x = m_xSeed->text().toInt();
//   int y = m_ySeed->text().toInt();
//   int z = m_zSeed->text().toInt();
//
//   if ( ROI[0] > x || ROI[1] < x
//     || ROI[2] > y || ROI[3] < y
//     || ROI[4] > z || ROI[5] < z )
//   {
//     QMessageBox::warning(this,
//                          tr("Seed Grow Segmentation"),
//                          tr("Segmentation couldn't be modified. Seed is outside ROI"));
//                          return;
//   }

  m_undoStack->beginMacro("Modify Seed GrowSegmentation Filter");
  {
    m_undoStack->push(new SGSFilterModification(m_filter, m_filter->roi(), m_gui->m_threshold->value(), m_gui->m_closeRadius->value()));
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

  m_viewManager->updateSegmentationRepresentations();
}
