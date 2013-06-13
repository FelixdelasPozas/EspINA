/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "CODEFilterInspector.h"
#include <Core/Filters/MorphologicalEditionFilter.h>
#include <GUI/ViewManager.h>
#include <Core/OutputRepresentations/VolumeRepresentation.h>

// Qt
#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QUndoCommand>
#include <QMessageBox>

using namespace EspINA;

class CODEModification
: public QUndoCommand
{
public:
  CODEModification(MorphologicalEditionFilter *filter,
                   int radius,
                   QUndoCommand *parent = NULL)
  : QUndoCommand(parent)
  , m_filter(filter)
  , m_radius(radius)
  {
    m_oldRadius = m_filter->radius();
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

    m_filter->setRadius(m_radius, m_newVolume.IsNotNull());

    if (m_newVolume.IsNull())
    {
      update();

      if (m_filter->isOutputEmpty())
        return;

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
    m_filter->setRadius(m_oldRadius, true);

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
  MorphologicalEditionFilter *m_filter;

  int m_radius, m_oldRadius;

  itkVolumeType::Pointer m_oldVolume;
  itkVolumeType::Pointer m_newVolume;

  FilterOutput::EditedRegionSList m_editedRegions;
};


//----------------------------------------------------------------------------
CODESettings::CODESettings(QString title,
                           MorphologicalEditionFilter* filter,
                           QUndoStack  *undoStack,
                           ViewManager *viewManager)
: QWidget()
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_filter(filter)
{
  QGroupBox *group = new QGroupBox(title, this);
  QLabel *label = new QLabel(tr("Radius"));
  m_spinbox = new QSpinBox();
  m_spinbox->setMinimum(0);
  m_spinbox->setMaximum(100);
  m_spinbox->setValue(filter->radius());

  QHBoxLayout *groupLayout = new QHBoxLayout();
  groupLayout->addWidget(label);
  groupLayout->addWidget(m_spinbox);
  group->setLayout(groupLayout);

  QPushButton *modifyButton = new QPushButton(tr("Modify"));
  QHBoxLayout *modifyLayout = new QHBoxLayout();
  modifyLayout->addStretch();
  modifyLayout->addWidget(modifyButton);
  modifyLayout->addStretch();

  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(group);
  mainLayout->addLayout(modifyLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  connect(modifyButton, SIGNAL(clicked(bool)),
          this, SLOT(modifyFilter()));
  connect(m_filter, SIGNAL(modified(ModelItemPtr)),
          this, SLOT(updateWidget()));

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  setMinimumWidth(150);
}

//----------------------------------------------------------------------------
CODESettings::~CODESettings()
{

}

//----------------------------------------------------------------------------
void CODESettings::modifyFilter()
{
  SegmentationOutputSPtr output = boost::dynamic_pointer_cast<SegmentationOutput>(m_filter->output(0));

  if ((m_spinbox->value() == static_cast<int>(m_filter->radius())) && !output->isEdited())
    return;

  if (output->isEdited())
  {
    QMessageBox msg;
    msg.setText(tr("Filter contains segmentations that have been manually modified by the user. "
                   "Updating will result in losing user modifications. "
                   "Do you want to proceed?"));
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);

    if (msg.exec() != QMessageBox::Yes)
      return;
  }

  CODEModification *filterModification = new CODEModification(m_filter, m_spinbox->value());
  filterModification->redo();
  filterModification->undo();

  if (m_filter->isOutputEmpty())
  {
    QMessageBox msg;
    msg.setText(tr("Operation with this radius will destroy the segmentation. "
		               "The modifying operation has been canceled."));
    msg.exec();
    delete filterModification;
    return;
  }

  m_undoStack->beginMacro("Modify Radius");
  m_undoStack->push(filterModification);
  m_undoStack->endMacro();
}

//----------------------------------------------------------------------------
void CODESettings::updateWidget()
{
  m_spinbox->setValue(m_filter->radius());
}

//----------------------------------------------------------------------------
CODEFilterInspector::CODEFilterInspector(QString title, MorphologicalEditionFilter *filter)
: m_title(title)
, m_filter(filter)
{
}

//----------------------------------------------------------------------------
QWidget *CODEFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
{
  return new CODESettings(m_title, m_filter, stack, viewManager);
}
