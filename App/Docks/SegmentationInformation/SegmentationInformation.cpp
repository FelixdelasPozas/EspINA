/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "SegmentationInformation.h"

#include <Core/Analysis/Segmentation.h>
#include "NoFilterRefiner.h"
#include <Support/FilterRefiner.h>
#include <Support/Utils/TagUtils.h>

#include "ui_SegmentationInformation.h"
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>

#include <QDebug>
#include <QLayout>
#include <QLabel>

using namespace ESPINA;
using namespace ESPINA::Support;

class SegmentationInformation::UI
: public QWidget
, public Ui::SegmentationInformation
{
public:
  UI()
  {
    setupUi(this);
  }
};

//----------------------------------------------------------------------------
SegmentationInformation::SegmentationInformation(FilterRefinerRegister &filterRefiners,
                                                 Context               &context)
: DockWidget(tr("Information"), context)
, m_register(filterRefiners)
, m_segmentation(nullptr)
, m_gui(new UI())
{
  setObjectName("Segmentation Information Panel");

  setWidget(m_gui);

  connect(m_gui->manageTags, SIGNAL(clicked(bool)),
          this,              SLOT(manageTags()));

}

//----------------------------------------------------------------------------
SegmentationInformation::~SegmentationInformation()
{
  reset();
}

//----------------------------------------------------------------------------
void SegmentationInformation::reset()
{
  hideInformation();
}

//----------------------------------------------------------------------------
void SegmentationInformation::showEvent(QShowEvent *event)
{
  ESPINA::DockWidget::showEvent(event);

  connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));


  onSelectionChanged(getSelectedSegmentations());
}

//----------------------------------------------------------------------------
void SegmentationInformation::hideEvent(QHideEvent* event)
{
  hideInformation();

  disconnect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
             this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

  ESPINA::DockWidget::hideEvent(event);
}

//----------------------------------------------------------------------------
void SegmentationInformation::onSelectionChanged(SegmentationAdapterList selection)
{
  Q_ASSERT(isVisible());

  auto validSelection = selection.size() == 1;

  if (validSelection)
  {
    showInformation(selection.first());
  }
  else
  {
    hideInformation();
  }

  m_gui->setEnabled(validSelection);
}

//----------------------------------------------------------------------------
void SegmentationInformation::onOutputModified()
{
  if (m_filter != m_segmentation->filter())
  {
    removeRefineWidget();
    addRefineWidget();
  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::manageTags()
{
  Q_ASSERT(m_segmentation);

  SegmentationAdapterList segmentations;

  segmentations << m_segmentation;

  Support::Utils::Tags::manageTags(segmentations, getUndoStack());

  showTags();
}

//----------------------------------------------------------------------------
void SegmentationInformation::onNotesModified()
{
  if (m_segmentation)
  {
    auto note = m_gui->notes->toPlainText();

    if (note.isEmpty())
    {
      safeDeleteExtension<SegmentationNotes>(m_segmentation);
    }
    else
    {
      auto extension = retrieveOrCreateExtension<SegmentationNotes>(m_segmentation->extensions());
      extension->setNotes(note);
    }
  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::showInformation(SegmentationAdapterPtr segmentation)
{
  // Update if segmentation are different
  if (segmentation != m_segmentation)
  {
    hideInformation();

    m_segmentation = segmentation;
    m_filter       = m_segmentation?m_segmentation->filter():nullptr;

    if (m_segmentation)
    {
      connect(m_segmentation, SIGNAL(outputModified()),
                 this,        SLOT(onOutputModified()));

      connect(m_gui->notes, SIGNAL(textChanged()),
              this,         SLOT(onNotesModified()));

      showSegmentationName();
      addRefineWidget();
      showTags();
      showNotes();
    }

  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::hideInformation()
{
  if (m_segmentation)
  {
    disconnect(m_segmentation, SIGNAL(outputModified()),
               this,           SLOT(onOutputModified()));

    disconnect(m_gui->notes, SIGNAL(textChanged()),
               this,         SLOT(onNotesModified()));

    clearSegmentationName();
    removeRefineWidget();
    clearTags();
    clearNotes();

    m_segmentation = nullptr;
    m_filter.reset();
  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::showSegmentationName()
{
  auto name = m_segmentation->data(Qt::DisplayRole).toString();

  m_gui->segmentation->setText(name);
}

//----------------------------------------------------------------------------
void SegmentationInformation::clearSegmentationName()
{
  m_gui->segmentation->clear();
}

//----------------------------------------------------------------------------
void SegmentationInformation::addRefineWidget()
{
  Q_ASSERT(m_segmentation);
  Q_ASSERT(m_filter);

  QWidget *widget = nullptr;

  try
  {
    widget = m_register.createRefineWidget(m_segmentation, context());
  }
  catch (...)
  {
    widget = new NoFilterRefiner();
  }

  m_gui->refineGroup->layout()->addWidget(widget);
}

//----------------------------------------------------------------------------
void SegmentationInformation::removeRefineWidget()
{
  auto layout = m_gui->refineGroup->layout();
  if (layout->count() == 1)
  {
  auto widget = layout->itemAt(0)->widget();

  Q_ASSERT(widget);
  layout->removeWidget(widget);

  delete widget;
  }
}
//----------------------------------------------------------------------------
void SegmentationInformation::showTags()
{
  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationTags::TYPE))
  {
    auto extension = retrieveExtension<SegmentationTags>(extensions);

    m_gui->tags->setText(extension->tags().join(", "));
  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::clearTags()
{
  m_gui->tags->clear();
}

//----------------------------------------------------------------------------
void SegmentationInformation::showNotes()
{
  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationNotes::TYPE))
  {
    auto extension = retrieveExtension<SegmentationNotes>(extensions);

    m_gui->notes->setText(extension->notes());
  }
}

//----------------------------------------------------------------------------
void SegmentationInformation::clearNotes()
{
  m_gui->notes->clear();
}
