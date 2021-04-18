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
#include "NoFilterRefiner.h"
#include <App/Utils/TagUtils.h>
#include <Core/Analysis/Segmentation.h>
#include <Support/FilterRefiner.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Issues/ItemIssues.h>
#include <Extensions/Notes/SegmentationNotes.h>

// Qt
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
  hideInformation();
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
    const auto notesText = m_gui->notes->toPlainText();

    // check for notes only containing spaces, tabs and newlines.
    auto it = std::find_if(notesText.constBegin(), notesText.constEnd(), [](const QChar &c) { return !c.isSpace(); });
    const auto isEmptyNote = (it == notesText.constEnd());

    // not using the undo command because in this case the QTextEdit provides its own undo/redo.
    if (notesText.isEmpty() || isEmptyNote)
    {
      safeDeleteExtension<SegmentationNotes>(m_segmentation);
    }
    else
    {
      auto notesExtension = retrieveOrCreateSegmentationExtension<SegmentationNotes>(m_segmentation, getContext().factory());
      notesExtension->setNotes(notesText);
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

    QList<std::pair<QString, QString>> names;

    for(auto connection: connections)
    {
      const auto color   = connection.item2->category() ? connection.item2->category()->color().name() : "black";
      const auto name    = connection.item2->data().toString();
      const auto pointer = QString::number(reinterpret_cast<unsigned long long>(connection.item2.get()));
      const auto text    = tr("<b><a href=\"A%1\" style=\"color: %2;\">%3</a></b> at point <a href=\"B%4\">%4</a>")
                              .arg(pointer).arg(color).arg(name).arg(connection.point.toString());

      names.push_back(std::make_pair(name, text));
    }

    auto pairLessThan = [](const std::pair<QString, QString> &lhs, const std::pair<QString, QString> &rhs)
    {
      auto lname = lhs.first;
      auto rname = rhs.first;
      auto lparts = lname.split(' ');
      auto rparts = rname.split(' ');

      // check same category
      if(lparts[0] != rparts[0])
      {
        return lname < rname;
      }

      // same category, check numbers
      QRegExp numExtractor("(\\d+)");
      numExtractor.setMinimal(false);

      if ((numExtractor.indexIn(lname) == -1) || (numExtractor.indexIn(rname) == -1))
      {
        return lname < rname;
      }

      // use the last number, we can't be sure that there is only one
      int pos      = 0;
      int numLeft  = 0;
      int numRight = 0;

      while ((pos = numExtractor.indexIn(lname, pos)) != -1)
      {
        numLeft = numExtractor.cap(1).toInt();
        pos += numExtractor.matchedLength();
      }

      pos = 0;
      while ((pos = numExtractor.indexIn(rname, pos)) != -1)
      {
        numRight = numExtractor.cap(1).toInt();
        pos += numExtractor.matchedLength();
      }

      if (numLeft == numRight)
      {
        return lname < rname;
      }

      return numLeft < numRight;
    };

    std::sort(names.begin(), names.end(), pairLessThan);

    auto insertOp = [&layout, this](const std::pair<QString, QString> &pair)
    {
      auto label = new QLabel{pair.second, this};
      label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
      label->setOpenExternalLinks(false);
      label->setTextFormat(Qt::RichText);
      connect(label, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkActivated(const QString &)));

      layout->addWidget(label);
    };
    std::for_each(names.constBegin(), names.constEnd(), insertOp);

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
  const auto linkText = link.mid(1);

  if(link.startsWith("A"))
  {
    // segmentation link
    bool ok{false};
    unsigned long long ptrDir = linkText.toULongLong(&ok);
    if(ok)
    {
      auto seg = reinterpret_cast<SegmentationAdapter *>(ptrDir);
      if(seg)
      {
        SegmentationAdapterList list;
        list << seg;
        getSelection()->set(list);

        getViewState().focusViewOn(centroid(seg->bounds()));
      }
    }
  }
  else
  {
    // connection point link
    NmVector3 point{linkText};

    getViewState().focusViewOn(point);
  }
}
