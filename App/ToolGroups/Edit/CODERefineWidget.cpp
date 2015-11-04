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

#include "CODERefineWidget.h"

#include "ui_CODERefineWidget.h"
#include <GUI/Widgets/Styles.h>

#include <QMessageBox>
#include <QUndoStack>
#include <QToolBar>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;

class CODEModification
: public QUndoCommand
{
public:
  CODEModification(SegmentationAdapterPtr         segmentation,
                   MorphologicalEditionFilterSPtr filter,
                   int radius,
                   QUndoCommand *parent = NULL)
  : QUndoCommand(parent)
  , m_segmentation(segmentation)
  , m_filter(filter)
  , m_radius(radius)
  {
    m_oldRadius = m_filter->radius();
  }

  //----------------------------------------------------------------------------
  virtual void redo()
  {
    auto output = m_filter->output(0);


    if (!m_oldVolume && (output->isEdited() /*|| volume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE*/))
    {
      auto volume = readLockVolume(output);
      m_oldBounds     = volume->bounds();
      m_oldVolume     = volume->itkImage();
      m_editedRegions = volume->editedRegions();
    }

    m_filter->setRadius(m_radius);

    update();

//     if (m_newVolume.IsNull())
//     {
//       update();
//
//       if (m_filter->isOutputEmpty())
//         return;
//
//       SegmentationVolumeSPtr newVolume = volume;
//       if (newVolume->volumeRegion().GetNumberOfPixels() < MAX_UNDO_SIZE)
//         m_newVolume = volume->cloneVolume();
//     }
//     else
//     {
//       volume->setVolume(m_newVolume);
//     }
//
//     output->clearEditedRegions();
    m_segmentation->invalidateRepresentations();
  }

  //----------------------------------------------------------------------------
  virtual void undo()
  {
    m_filter->setRadius(m_oldRadius);

    if (m_oldVolume.IsNotNull())
    {
      auto output = m_filter->output(0);
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

private:
  void update()
  {
    WaitingCursor cursor;

    m_filter->update();
  }

  void invalidateRepresentations()
  {
    m_segmentation->invalidateRepresentations();
  }

private:
  SegmentationAdapterPtr         m_segmentation;
  MorphologicalEditionFilterSPtr m_filter;

  int m_radius, m_oldRadius;

  Bounds m_oldBounds;
  itkVolumeType::Pointer m_oldVolume;
  BoundsList             m_editedRegions;
  //itkVolumeType::Pointer m_newVolume;
};
//----------------------------------------------------------------------------
CODERefineWidget::CODERefineWidget(const QString                 &title,
                                   SegmentationAdapterPtr         segmentation,
                                   MorphologicalEditionFilterSPtr filter,
                                   Support::Context              &context)
: WithContext(context)
, m_gui(new Ui::CODERefineWidget())
, m_title(title)
, m_segmentation(segmentation)
, m_filter(filter)
{
  m_gui->setupUi(this);

  m_gui->titleLabel->setText(m_title + ":");
  m_gui->radius->setValue(m_filter->radius());

  connect(m_gui->radius, SIGNAL(valueChanged(int)),
          this,          SLOT(onRadiusChanged(int)));
  connect(m_gui->apply,  SIGNAL(clicked(bool)),
          this,          SLOT(refineFilter()));

  connect(m_segmentation->output().get(), SIGNAL(modified()),
          this,                           SLOT(onFilterModified()));
}

//----------------------------------------------------------------------------
CODERefineWidget::~CODERefineWidget()
{
}

//----------------------------------------------------------------------------
void CODERefineWidget::onRadiusChanged(int value)
{
  m_gui->apply->setEnabled(true);

  emit radiusChanged(value);
}

//----------------------------------------------------------------------------
void CODERefineWidget::onFilterModified()
{
  m_gui->radius->setValue(m_filter->radius());
}

//----------------------------------------------------------------------------
void CODERefineWidget::refineFilter()
{
  auto output = m_filter->output(0);

  if (output->isEdited())
  {
    auto buttons = QMessageBox::Yes|QMessageBox::Cancel;
    auto message = tr("Filter contains segmentations that have been manually modified by the user."
                  "Updating this filter will result in losing user modifications."
                  "Do you want to proceed?");

    if (GUI::DefaultDialogs::UserQuestion(message, buttons, m_title) == QMessageBox::Cancel) return;
  }

  auto volume  = readLockVolume(output);

  auto undoStack = getUndoStack();

  undoStack->beginMacro(tr("Modify %1 radius").arg(m_title));
  {
    undoStack->push(new CODEModification(m_segmentation, m_filter, m_gui->radius->value()));
  }
  undoStack->endMacro();
}

//----------------------------------------------------------------------------
void CODERefineWidget::setRadius(int value)
{
  m_gui->radius->setValue(value);
}
