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
#include "CODERefineWidget.h"
#include <Core/Utils/SignalBlocker.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Dialogs/DefaultDialogs.h>

#include "ui_CODERefineWidget.h"
#include <QMessageBox>
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;

//----------------------------------------------------------------------------
CODEModification::CODEModification(SegmentationAdapterPtr segmentation,
                                   unsigned int           radius,
                                   QUndoCommand          *parent)
: QUndoCommand  {parent}
, m_segmentation{segmentation}
, m_filter      {std::dynamic_pointer_cast<MorphologicalEditionFilter>(segmentation->filter())}
, m_radius      {radius}
, m_oldRadius   {m_filter->radius()}
{
  Q_ASSERT(m_filter != nullptr);
}

//----------------------------------------------------------------------------
void CODEModification::redo()
{
  auto output = m_filter->output(0);

  if (!m_oldVolume && output->isEdited())
  {
    auto volume = readLockVolume(output);
    m_oldBounds = volume->bounds();
    m_oldVolume = volume->itkImage();
    m_editedRegions = volume->editedRegions();
  }

  m_filter->setRadius(m_radius);

  update();

  invalidateRepresentations();
}

//----------------------------------------------------------------------------
void CODEModification::undo()
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
//----------------------------------------------------------------------------
void CODEModification::update()
{
  WaitingCursor cursor;

  m_filter->update();
}

//----------------------------------------------------------------------------
void CODEModification::invalidateRepresentations()
{
  m_segmentation->invalidateRepresentations();
}

//----------------------------------------------------------------------------
CODERefineWidget::CODERefineWidget(const QString                 &title,
                                   SegmentationAdapterPtr         segmentation,
                                   Support::Context              &context,
                                   QWidget                       *parent)
: QWidget       {parent}
, WithContext   (context)
, m_gui         {new Ui::CODERefineWidget()}
, m_title       {title}
, m_segmentation{segmentation}
, m_filter      {std::dynamic_pointer_cast<MorphologicalEditionFilter>(segmentation->filter())}
{
  m_gui->setupUi(this);

  m_gui->titleLabel->setText(m_title + ":");
  m_gui->radius->setValue(m_filter->radius());
  m_gui->apply->setEnabled(false);

  connect(m_gui->apply,  SIGNAL(clicked(bool)),
          this,          SLOT(refineFilter()));

  connect(m_filter.get(), SIGNAL(radiusModified(int)),
          this,           SLOT(onRadiusModified(int)));
}

//----------------------------------------------------------------------------
CODERefineWidget::~CODERefineWidget()
{
  if(m_gui->apply->isEnabled())
  {
    auto answer = GUI::DefaultDialogs::UserQuestion(tr("The properties of the segmentation '%1' have been modified but haven't been applied, do you want to discard them?").arg(m_segmentation->data().toString()),
                                                    QMessageBox::Apply|QMessageBox::Discard,
                                                    m_title);

    if(answer == QMessageBox::Apply)
    {
      refineFilter();
    }
  }
}

//----------------------------------------------------------------------------
void CODERefineWidget::onRadiusModified(int value)
{
  m_gui->radius->setValue(value);
  m_gui->apply->setEnabled(value == static_cast<int>(m_filter->radius()));
}

//----------------------------------------------------------------------------
void CODERefineWidget::refineFilter()
{
  auto output = m_filter->output(0);

  SignalBlocker<OutputSPtr> blocker(output);

  if (output->isEdited())
  {
    auto buttons = QMessageBox::Yes|QMessageBox::Cancel;
    auto message = tr("Filter contains segmentations that have been manually modified by the user."
                  "Updating this filter will result in losing user modifications."
                  "Do you want to proceed?");

    if (GUI::DefaultDialogs::UserQuestion(message, buttons, m_title) == QMessageBox::Cancel) return;
  }

  {
    WaitingCursor cursor;

    auto undoStack   = getUndoStack();
    auto radiusValue = m_gui->radius->value();
    undoStack->beginMacro(tr("Modify %1 radius of '%2' to %3.").arg(m_title).arg(m_segmentation->data().toString()).arg(radiusValue));
    {
      undoStack->push(new CODEModification(m_segmentation, static_cast<unsigned int>(radiusValue)));
    }
    undoStack->endMacro();
  }


  m_gui->apply->setEnabled(false);
}
