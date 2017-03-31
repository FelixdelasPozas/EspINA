/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/IssueList/IssueListDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI;

  //------------------------------------------------------------------------
IssueListDialog::IssueListDialog(IssueList issuesList)
: QDialog(DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint})
{
  setupUi(this);
  setWindowTitle(tr("Current session problems"));
  setWindowIcon(QIcon(":/espina/checklist.svg"));
  m_issueTable->setAlternatingRowColors(true);
  m_issueTable->setRowCount(issuesList.size());
  m_issueTable->setColumnCount(4);
  m_issueTable->setSortingEnabled(true);

  QStringList headerLabels;
  headerLabels << tr("Severity");
  headerLabels << tr("Element");
  headerLabels << tr("Problem");
  headerLabels << tr("Suggestion");

  m_issueTable->setHorizontalHeaderLabels(headerLabels);

  int row = 0;
  for(auto issue: issuesList)
  {
    auto itemWidget = new IssueTableWidgetItem(issue->displayName());
    itemWidget->setFlags(itemWidget->flags() ^ Qt::ItemIsEditable);

    auto severityWidget = new QTableWidgetItem();
    switch(issue->severity())
    {
      case Issue::Severity::CRITICAL:
        severityWidget->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));
        severityWidget->setData(Qt::DisplayRole, tr("Critical"));
        break;
      case Issue::Severity::WARNING:
        severityWidget->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
        severityWidget->setData(Qt::DisplayRole, tr("Warning"));
        break;
      case Issue::Severity::INFORMATION:
        severityWidget->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
        severityWidget->setData(Qt::DisplayRole, tr("Information"));
        break;
      case Issue::Severity::NONE:
        severityWidget->setData(Qt::DisplayRole, tr("None"));
        break;
      default:
        break;
    }
    severityWidget->setFlags(severityWidget->flags() ^ Qt::ItemIsEditable);


    auto descriptionWidget = new QTableWidgetItem(issue->description());
    descriptionWidget->setFlags(descriptionWidget->flags() ^ Qt::ItemIsEditable);

    auto suggestionWidget = new QTableWidgetItem(issue->suggestion());
    suggestionWidget->setFlags(suggestionWidget->flags() ^ Qt::ItemIsEditable);

    m_issueTable->setItem(row, 0, severityWidget);
    m_issueTable->setItem(row, 1, itemWidget);
    m_issueTable->setItem(row, 2, descriptionWidget);
    m_issueTable->setItem(row, 3, suggestionWidget);
    ++row;
  }

  m_issueTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_issueTable->horizontalHeader()->setStretchLastSection(true);
  m_issueTable->adjustSize();
  m_issueTable->sortByColumn(0, Qt::AscendingOrder);
  m_issueTable->horizontalHeader()->setSortIndicatorShown(true);
}

//------------------------------------------------------------------------
IssueListDialog::~IssueListDialog()
{
}
