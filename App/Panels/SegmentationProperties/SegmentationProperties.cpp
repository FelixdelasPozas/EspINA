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

// ESPINA
#include "ui_SegmentationProperties.h"
#include "SegmentationProperties.h"
#include <App/Utils/TagUtils.h>
#include <Core/Analysis/Segmentation.h>
#include "NoFilterRefiner.h"
#include <Support/FilterRefiner.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <Extensions/Issues/SegmentationIssues.h>

// Qt
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QPixmap>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Support;
using namespace ESPINA::Extensions;

//----------------------------------------------------------------------------
class SegmentationProperties::UI
: public QWidget
, public Ui::SegmentationProperties
{
  public:
    UI(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
    : QWidget{parent, f}
    {
      setupUi(this);
    }
};

//----------------------------------------------------------------------------
SegmentationProperties::SegmentationProperties(FilterRefinerFactory &filterRefiners, Context &context, QWidget *parent)
: Panel         {tr("Segmentation Properties"), context, parent}
, m_register    (filterRefiners)
, m_segmentation{nullptr}
, m_gui         {new UI(this)}
{
  setObjectName("SegmentationProperties");

  setWidget(m_gui);

  m_gui->issuesGroup->hide();
  m_gui->connectionsGroup->hide();

  connect(m_gui->manageTags, SIGNAL(clicked(bool)), this, SLOT(manageTags()));
}

//----------------------------------------------------------------------------
SegmentationProperties::~SegmentationProperties()
{
  reset();
}

//----------------------------------------------------------------------------
void SegmentationProperties::reset()
{
  hideInformation();
}

//----------------------------------------------------------------------------
void SegmentationProperties::showEvent(QShowEvent *event)
{
  ESPINA::Panel::showEvent(event);

  connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

  onSelectionChanged(getSelectedSegmentations());
}

//----------------------------------------------------------------------------
void SegmentationProperties::hideEvent(QHideEvent* event)
{
  hideInformation();

  disconnect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
             this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

  ESPINA::Panel::hideEvent(event);
}

//----------------------------------------------------------------------------
void SegmentationProperties::onSelectionChanged(SegmentationAdapterList selection)
{
  Q_ASSERT(isVisible());

  auto validSelection = (selection.size() == 1);

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
void SegmentationProperties::onOutputModified()
{
  if (m_filter != m_segmentation->filter())
  {
    removeRefineWidget();
    addRefineWidget();
  }
  else
  {
    showConnections();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::manageTags()
{
  Q_ASSERT(m_segmentation);

  SegmentationAdapterList segmentations;

  segmentations << m_segmentation;

  manageTagsDialog(segmentations, getUndoStack(), getContext().factory().get());

  showTags();
}

//----------------------------------------------------------------------------
void SegmentationProperties::onNotesModified()
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
      auto notesExtension = retrieveOrCreateSegmentationExtension<SegmentationNotes>(m_segmentation, getContext().factory());
      notesExtension->setNotes(note);
    }
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::showInformation(SegmentationAdapterPtr segmentation)
{
  // Update if segmentation are different
  if (segmentation != m_segmentation)
  {
    hideInformation();

    m_segmentation = segmentation;
    m_filter = m_segmentation ? m_segmentation->filter() : nullptr;

    if (m_segmentation)
    {
      showSegmentationName();
      addRefineWidget();
      showTags();
      showNotes();
      showIssues();
      showConnections();

      connect(m_segmentation, SIGNAL(outputModified()),
              this,           SLOT(onOutputModified()));

      connect(m_gui->notes, SIGNAL(textChanged()),
              this,         SLOT(onNotesModified()));
    }
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::hideInformation()
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
    clearIssues();
    clearConnections();

    m_segmentation = nullptr;
    m_filter.reset();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::showSegmentationName()
{
  auto name = m_segmentation->data(Qt::DisplayRole).toString();

  m_gui->segmentation->setText(name);
}

//----------------------------------------------------------------------------
void SegmentationProperties::clearSegmentationName()
{
  m_gui->segmentation->clear();
}

//----------------------------------------------------------------------------
void SegmentationProperties::addRefineWidget()
{
  Q_ASSERT(m_segmentation);
  Q_ASSERT(m_filter);

  try
  {
    m_gui->refineGroup->layout()->addWidget(m_register.createRefineWidget(m_segmentation, getContext(), m_gui->refineGroup));
  }
  catch (...)
  {
    m_gui->refineGroup->layout()->addWidget(new NoFilterRefiner(m_gui->refineGroup));
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::removeRefineWidget()
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
void SegmentationProperties::showTags()
{
  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationTags::TYPE))
  {
    auto extension = retrieveExtension<SegmentationTags>(extensions);
    auto tags = extension->tags();

    tags.removeDuplicates();
    m_gui->tags->setText(tags.join(", "));
  }
  else
  {
    clearTags();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::clearTags()
{
  m_gui->tags->clear();
}

//----------------------------------------------------------------------------
void SegmentationProperties::showNotes()
{
  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationNotes::TYPE))
  {
    auto extension = retrieveExtension<SegmentationNotes>(extensions);

    m_gui->notes->setText(extension->notes());
  }
  else
  {
    clearNotes();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::clearNotes()
{
  m_gui->notes->clear();
}

//----------------------------------------------------------------------------
void SegmentationProperties::showIssues()
{
  clearIssues();

  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationIssues::TYPE))
  {
    IssueProperty *issueProperty = nullptr;
    auto extension = retrieveExtension<SegmentationIssues>(extensions);
    auto isueList = extension->issues();
    QString icon_path;
    QPixmap icon;
    QSize size(24, 24);

    for (auto issue : isueList)
    {
      switch (issue->severity())
      {
        case Issue::Severity::WARNING:
          icon_path = SegmentationIssues::severityIcon(Issue::Severity::WARNING, true);
          icon = QIcon(icon_path).pixmap(size);
          issueProperty = new IssueProperty(issue->description(), issue->suggestion(), m_gui->issuesGroup, icon);
          break;
        case Issue::Severity::CRITICAL:
          icon_path = SegmentationIssues::severityIcon(Issue::Severity::CRITICAL, true);
          icon = QIcon(icon_path).pixmap(size);
          issueProperty = new IssueProperty(issue->description(), issue->suggestion(), m_gui->issuesGroup, icon);
          break;
        default:
          issueProperty = new IssueProperty(issue->description(), issue->suggestion(), m_gui->issuesGroup);
          break;
      }

      if (issueProperty != nullptr)
      {
        m_gui->issuesGroup->layout()->addWidget(issueProperty);
      }
    }

    m_gui->issuesGroup->show();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::clearIssues()
{
  auto layout = m_gui->issuesGroup->layout();
  while (layout->count() != 0)
  {
    auto widget = layout->itemAt(0)->widget();

    Q_ASSERT(widget);
    layout->removeWidget(widget);

    delete widget;
  }

  m_gui->issuesGroup->hide();
}

//----------------------------------------------------------------------------
void SegmentationProperties::showConnections()
{
  clearConnections();

  auto segmentation = getModel()->smartPointer(m_segmentation);
  auto connections  = getModel()->connections(segmentation);

  if(!connections.isEmpty())
  {
    auto layout = m_gui->connectionsGroup->layout();
    for(auto connection: connections)
    {
      auto text = tr("<b>%1</b> at point <a href=""%2"">%2</a>").arg(connection.item2->data().toString()).arg(connection.point.toString());
      auto label = new QLabel{text};
      label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
      label->setOpenExternalLinks(false);
      label->setTextFormat(Qt::RichText);
      connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));

      layout->addWidget(label);
    }

    m_gui->connectionsGroup->show();
  }
}

//----------------------------------------------------------------------------
void SegmentationProperties::clearConnections()
{
  auto layout = m_gui->connectionsGroup->layout();
  while (layout->count() != 0)
  {
    auto widget = layout->itemAt(0)->widget();

    Q_ASSERT(widget);
    layout->removeWidget(widget);

    delete widget;
  }

  if(m_gui->connectionsGroup->isVisible()) m_gui->connectionsGroup->hide();
}

//--------------------------------------------------------------------
void SegmentationProperties::onLinkActivated(const QString &link)
{
  NmVector3 point{link};

  getViewState().focusViewOn(point);
}
