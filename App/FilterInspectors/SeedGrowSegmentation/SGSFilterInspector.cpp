/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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
#include "SGSFilterInspector.h"
#include <Undo/VolumeSnapshotCommand.h>

// EspINA
#include <Core/Filters/SeedGrowSegmentationFilter.h>
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/RectangularRegion.h>
#include <Core/OutputRepresentations/VolumeRepresentation.h>

// Qt
#include <QDebug>
#include <QMessageBox>
#include <QCheckBox>
#include <QUndoCommand>

using namespace EspINA;

class SGSFilterModification
: public QUndoCommand
{
public:
  SGSFilterModification(SeedGrowSegmentationFilter *filter,
                        int voi[6],
                        int threshold,
                        int closeRadius,
                        QUndoCommand *parent = NULL)
  : QUndoCommand(parent)
  , m_filter(filter)
  , m_threshold(threshold)
  , m_closeRadius(closeRadius)
  {
    memcpy(m_VOI, voi, 6*sizeof(int));

    m_oldThreshold = m_filter->lowerThreshold();
    m_filter->voi(m_oldVOI);
    m_oldCloseRadius = m_filter->closeValue();
  }

  virtual void redo()
  {
    SegmentationOutputSPtr output = boost::dynamic_pointer_cast<SegmentationOutput>(m_filter->output(0));

    SegmentationVolumeSPtr volume = segmentationVolume(output);

    if (!m_oldVolume && (output->isEdited() || volume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE))
    {
      m_oldVolume     = volume->cloneVolume();
      m_editedRegions = output->editedRegions();
    }

    bool ignoreUpdate = m_newVolume.IsNotNull();

    m_filter->setLowerThreshold(m_threshold, ignoreUpdate);
    m_filter->setUpperThreshold(m_threshold, ignoreUpdate);
    m_filter->setVOI(m_VOI, ignoreUpdate);
    m_filter->setCloseValue(m_closeRadius, ignoreUpdate);

    if (m_newVolume.IsNull())
    {
      update();

      SegmentationVolumeSPtr newVolume = volume;
      if (newVolume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE)
        m_newVolume = volume->cloneVolume();
    }
    else
    {
      volume->setVolume(m_newVolume);
    }

    output->clearEditedRegions();
  }

  virtual void undo()
  {
    m_filter->setLowerThreshold(m_oldThreshold, true);
    m_filter->setUpperThreshold(m_oldThreshold, true);
    m_filter->setVOI(m_oldVOI, true);
    m_filter->setCloseValue(m_oldCloseRadius, true);

    SegmentationOutputSPtr output = boost::dynamic_pointer_cast<SegmentationOutput>(m_filter->output(0));
    SegmentationVolumeSPtr volume = segmentationVolume(output);

    if (m_oldVolume.IsNotNull())
    {
      volume->setVolume(m_oldVolume);
      output->setEditedRegions(m_editedRegions);
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
  SeedGrowSegmentationFilter *m_filter;

  int m_VOI[6], m_oldVOI[6];
  int m_threshold, m_oldThreshold;
  int m_closeRadius, m_oldCloseRadius;

  itkVolumeType::Pointer m_oldVolume;
  itkVolumeType::Pointer m_newVolume;

  FilterOutput::EditedRegionSList m_editedRegions;
};

//----------------------------------------------------------------------------
SGSFilterInspector::SGSFilterInspector(SeedGrowSegmentationFilter* filter)
: m_filter(filter)
{
}

//----------------------------------------------------------------------------
QWidget* SGSFilterInspector::createWidget(QUndoStack* stack, ViewManager* viewManager)
{
  return new Widget(m_filter, stack, viewManager);
}




//----------------------------------------------------------------------------
SGSFilterInspector::Widget::Widget(Filter* filter,
                                   QUndoStack * undoStack,
                                   ViewManager* viewManager)
: m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_region(NULL)
, m_closeValue(0)
//, m_sliceSelctor(NULL)
{
  setupUi(this);
  m_filter = dynamic_cast<SeedGrowSegmentationFilter *>(filter);
  connect(filter, SIGNAL(modified(ModelItemPtr)),
          this, SLOT(updateWidget()));

  SegmentationVolumeSPtr volume = segmentationVolume(filter->output(0));

  itkVolumeType::IndexType seed = m_filter->seed();
  m_xSeed->setText(QString("%1").arg(seed[0]));
  m_ySeed->setText(QString("%1").arg(seed[1]));
  m_zSeed->setText(QString("%1").arg(seed[2]));
  m_threshold->setMaximum(255);
  m_threshold->setValue(m_filter->lowerThreshold());
  int voiExtent[6];
  m_filter->voi(voiExtent);
  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  for (int i=0; i<6; i++)
    m_voiBounds[i] = voiExtent[i] * spacing[i/2];

  m_leftMargin->setValue(m_voiBounds[0]);
  m_leftMargin->setSuffix(" nm");
  m_leftMargin->installEventFilter(this);
  connect(m_leftMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_rightMargin->setValue(m_voiBounds[1]);
  m_rightMargin->setSuffix(" nm");
  m_rightMargin->installEventFilter(this);
  connect(m_rightMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_topMargin->setValue(m_voiBounds[2]);
  m_topMargin->setSuffix(" nm");
  m_topMargin->installEventFilter(this);
  connect(m_topMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_bottomMargin->setValue(m_voiBounds[3]);
  m_bottomMargin->setSuffix(" nm");
  m_bottomMargin->installEventFilter(this);
  connect(m_bottomMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds( )));

  m_upperMargin->setValue(m_voiBounds[4]);
  m_upperMargin->setSuffix(" nm");
  m_upperMargin->installEventFilter(this);
  connect(m_upperMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  m_lowerMargin->setValue(m_voiBounds[5]);
  m_lowerMargin->setSuffix(" nm"); 
  m_lowerMargin->installEventFilter(this);
  connect(m_lowerMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateRegionBounds()));

  bool enabled = m_filter->closeValue() > 0;
  m_closeCheckbox->setChecked(enabled);
  connect(m_closeCheckbox, SIGNAL(stateChanged(int)),
      this, SLOT(modifyCloseCheckbox(int)));

  m_closeRadius->setEnabled(enabled);
  m_closeRadius->setValue(m_filter->closeValue());
  connect(m_closeRadius, SIGNAL(valueChanged(int)),
      this, SLOT(modifyCloseValue(int)));


  connect(m_modify, SIGNAL(clicked(bool)),
          this, SLOT(modifyFilter()));
}

//----------------------------------------------------------------------------
SGSFilterInspector::Widget::~Widget()
{
  if (m_region)
  {
    m_viewManager->removeWidget(m_region);
    delete m_region;
    //m_viewManager->removeSliceSelectors(m_sliceSelctor);
    //delete m_sliceSelctor;
  }
}

//----------------------------------------------------------------------------
bool SGSFilterInspector::Widget::eventFilter(QObject* sender, QEvent* e)
{
  if (e->type() == QEvent::FocusIn)
  {
    if (!m_region)
    {
      m_region = new RectangularRegion(m_voiBounds, m_viewManager);
      connect(m_region, SIGNAL(modified(double*)),
              this, SLOT(redefineVOI(double*)));
      m_viewManager->addWidget(m_region);
      //m_sliceSelctor = new RectangularRegionSliceSelector(m_region);
      //m_sliceSelctor->setLeftLabel("SVOI");
      //m_sliceSelctor->setRightLabel("SVOI");
      //m_viewManager->addSliceSelectors(m_sliceSelctor, ViewManager::From|ViewManager::To);
      m_viewManager->updateViews();
    }
  }

  return QObject::eventFilter(sender, e);
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::redefineVOI(double* bounds)
{
  m_leftMargin  ->blockSignals(true);
  m_rightMargin ->blockSignals(true);
  m_topMargin   ->blockSignals(true);
  m_bottomMargin->blockSignals(true);
  m_upperMargin ->blockSignals(true);
  m_lowerMargin ->blockSignals(true);

  m_leftMargin  ->setValue(bounds[0]);
  m_rightMargin ->setValue(bounds[1]);
  m_topMargin   ->setValue(bounds[2]);
  m_bottomMargin->setValue(bounds[3]);
  m_upperMargin ->setValue(bounds[4]);
  m_lowerMargin ->setValue(bounds[5]);

  m_leftMargin  ->blockSignals(false);
  m_rightMargin ->blockSignals(false);
  m_topMargin   ->blockSignals(false);
  m_bottomMargin->blockSignals(false);
  m_upperMargin ->blockSignals(false);
  m_lowerMargin ->blockSignals(false);
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::modifyFilter()
{
  SegmentationOutputSPtr output = boost::dynamic_pointer_cast<SegmentationOutput>(m_filter->output(0));

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

  SegmentationVolumeSPtr volume = segmentationVolume(output);

  double spacing[3];
  volume->spacing(spacing);

  double voiBounds[6];
  voiBounds[0] = m_leftMargin->value();
  voiBounds[1] = m_rightMargin->value();
  voiBounds[2] = m_topMargin->value();
  voiBounds[3] = m_bottomMargin->value();
  voiBounds[4] = m_upperMargin->value();
  voiBounds[5] = m_lowerMargin->value();

  int VOI[6];
  for(int i=0; i < 6; i++)
    VOI[i] = voiBounds[i]/spacing[i/2];

  int x = m_xSeed->text().toInt();
  int y = m_ySeed->text().toInt();
  int z = m_zSeed->text().toInt();

  if ( VOI[0] > x || VOI[1] < x
    || VOI[2] > y || VOI[3] < y
    || VOI[4] > z || VOI[5] < z )
  {
    QMessageBox::warning(this,
                         tr("Seed Grow Segmentation"),
                         tr("Segmentation couldn't be modified. Seed is outside VOI"));
                         return;
  }

  m_undoStack->beginMacro("Modify Seed GrowSegmentation Filter");
  {
    m_undoStack->push(new SGSFilterModification(m_filter, VOI, m_threshold->value(), m_closeValue));
  }
  m_undoStack->endMacro();

  if (m_filter->isTouchingVOI())
  {
    QMessageBox warning;
    warning.setIcon(QMessageBox::Warning);
    warning.setWindowTitle(tr("Seed Grow Segmentation Filter Information"));
    warning.setText(tr("New segmentation may be incomplete due to VOI restriction."));
    warning.exec();
  }
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::updateRegionBounds()
{
  m_voiBounds[0] = m_leftMargin->value();
  m_voiBounds[1] = m_rightMargin->value();
  m_voiBounds[2] = m_topMargin->value();
  m_voiBounds[3] = m_bottomMargin->value();
  m_voiBounds[4] = m_upperMargin->value();
  m_voiBounds[5] = m_lowerMargin->value();

  if (m_region)
    m_region->setBounds(m_voiBounds);
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::modifyCloseCheckbox(int enable)
{
  m_closeRadius->setEnabled(enable);

  if (!enable)
    m_closeValue = 0;
  else
    m_closeValue = m_closeRadius->value();
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::modifyCloseValue(int value)
{
  m_closeValue = value;
}

//----------------------------------------------------------------------------
void SGSFilterInspector::Widget::updateWidget()
{
  SegmentationVolumeSPtr volume = segmentationVolume(m_filter->output(0));

  m_threshold->setValue(m_filter->lowerThreshold());
  int voiExtent[6];
  m_filter->voi(voiExtent);
  itkVolumeType::SpacingType spacing = volume->toITK()->GetSpacing();
  for (int i=0; i<6; i++)
    m_voiBounds[i] = voiExtent[i] * spacing[i/2];

  m_leftMargin->setValue(m_voiBounds[0]);
  m_rightMargin->setValue(m_voiBounds[1]);
  m_topMargin->setValue(m_voiBounds[2]);
  m_bottomMargin->setValue(m_voiBounds[3]);
  m_upperMargin->setValue(m_voiBounds[4]);
  m_lowerMargin->setValue(m_voiBounds[5]);

  bool enabled = m_filter->closeValue() > 0;
  m_closeCheckbox->setChecked(enabled);
  m_closeRadius->setEnabled(enabled);
  m_closeRadius->setValue(m_filter->closeValue());
}
