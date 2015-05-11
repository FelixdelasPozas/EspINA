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

namespace ESPINA
{
  //------------------------------------------------------------------------
  IssueListDialog::IssueListDialog(IssueList issuesList)
  {
    setupUi(this);
    setWindowTitle(tr("Current session problems"));
    m_issueTable->setAlternatingRowColors(true);
    m_issueTable->setRowCount(issuesList.size());
    m_issueTable->setColumnCount(4);
    m_issueTable->setSortingEnabled(true);
    QStringList headerLabels;
    headerLabels << tr("Element");
    headerLabels << tr("Severity");
    headerLabels << tr("Problem");
    headerLabels << tr("Suggestion");
    m_issueTable->setHorizontalHeaderLabels(headerLabels);

    int row = 0;
    for(auto problem: issuesList)
    {
      IssueTableWidgetItem *item = new IssueTableWidgetItem(problem.element);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);

      QTableWidgetItem *severity = new QTableWidgetItem();
      switch(problem.severity)
      {
        case Severity::CRITICAL:
          severity->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));
          severity->setData(Qt::DisplayRole, tr("Critical"));
          break;
        case Severity::WARNING:
          severity->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
          severity->setData(Qt::DisplayRole, tr("Warning"));
          break;
        case Severity::INFORMATION:
          severity->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
          severity->setData(Qt::DisplayRole, tr("Information"));
          break;
        default:
          break;
      }

      QTableWidgetItem *description = new QTableWidgetItem(problem.message);
      description->setFlags(description->flags() ^ Qt::ItemIsEditable);
      QTableWidgetItem *suggestion = new QTableWidgetItem(problem.suggestion);
      suggestion->setFlags(suggestion->flags() ^ Qt::ItemIsEditable);

      m_issueTable->setItem(row, 0, item);
      m_issueTable->setItem(row, 1, severity);
      m_issueTable->setItem(row, 2, description);
      m_issueTable->setItem(row, 3, suggestion);
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

} // namespace ESPINA
