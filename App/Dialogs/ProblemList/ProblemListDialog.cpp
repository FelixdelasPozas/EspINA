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
#include "ProblemListDialog.h"

namespace ESPINA
{
  //------------------------------------------------------------------------
  ProblemListDialog::ProblemListDialog(ProblemList problemList)
  {
    setupUi(this);
    setWindowTitle(tr("Current session problems"));
    m_problemTable->setAlternatingRowColors(true);
    m_problemTable->setRowCount(problemList.size());
    m_problemTable->setColumnCount(4);
    m_problemTable->setSortingEnabled(true);
    QStringList headerLabels;
    headerLabels << tr("Element");
    headerLabels << tr("Severity");
    headerLabels << tr("Problem");
    headerLabels << tr("Suggestion");
    m_problemTable->setHorizontalHeaderLabels(headerLabels);

    int row = 0;
    for(auto problem: problemList)
    {
      ProblemTableWidgetItem *item = new ProblemTableWidgetItem(problem.element);
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

      m_problemTable->setItem(row, 0, item);
      m_problemTable->setItem(row, 1, severity);
      m_problemTable->setItem(row, 2, description);
      m_problemTable->setItem(row, 3, suggestion);
      ++row;
    }

    m_problemTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_problemTable->horizontalHeader()->setStretchLastSection(true);
    m_problemTable->adjustSize();
    m_problemTable->sortByColumn(0, Qt::AscendingOrder);
    m_problemTable->horizontalHeader()->setSortIndicatorShown(true);
  }

  //------------------------------------------------------------------------
  ProblemListDialog::~ProblemListDialog()
  {
  }

} // namespace ESPINA
