/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#include "ProblemListDialog.h"
#include <QDebug>

namespace EspINA
{
  //------------------------------------------------------------------------
  ProblemListDialog::ProblemListDialog(ProblemList problemList)
  {
    setupUi(this);
    setWindowTitle(tr("Current session problems"));

    m_problemTable->setAlternatingRowColors(true);
    m_problemTable->setRowCount(problemList.size());
    m_problemTable->setColumnCount(3);
    m_problemTable->setSortingEnabled(true);
    QStringList headerLabels;
    headerLabels << tr("Element");
    headerLabels << tr("Problem");
    headerLabels << tr("Suggestion");
    m_problemTable->setHorizontalHeaderLabels(headerLabels);
    m_problemTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    m_problemTable->horizontalHeader()->setStretchLastSection(true);
    
    int row = 0;
    for(auto problem: problemList)
    {
      ProblemTableWidgetItem *item = new ProblemTableWidgetItem(problem.element);
      switch(problem.severity)
      {
        case Severity::CRITICAL:
          item->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));
          break;
        case Severity::WARNING:
          item->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
          break;
        case Severity::INFORMATION:
          item->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
          break;
        default:
          break;
      }

      m_problemTable->setItem(row, 0, item);
      QTableWidgetItem *description = new QTableWidgetItem(problem.message);
      m_problemTable->setItem(row, 1, description);
      QTableWidgetItem *suggestion = new QTableWidgetItem(problem.suggestion);
      m_problemTable->setItem(row, 2, suggestion);
      ++row;
    }

    m_problemTable->sortByColumn(0, Qt::AscendingOrder);
    m_problemTable->horizontalHeader()->setSortIndicatorShown(true);
  }

  //------------------------------------------------------------------------
  ProblemListDialog::~ProblemListDialog()
  {
  }

} // namespace EspINA
