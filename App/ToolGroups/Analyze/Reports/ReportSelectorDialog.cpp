/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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
#include "ReportSelectorDialog.h"

#include <Support/Report.h>

using namespace ESPINA;

//------------------------------------------------------------------------
ReportSelectorDialog::ReportSelectorDialog(Support::ReportSList &reports)
: m_reports(reports)
{
  setupUi(this);
  setWindowTitle(tr("Report Selector"));

  connect(this, SIGNAL(accepted()),
          this, SLOT(showReport()));

  connect(m_list, SIGNAL(currentRowChanged(int)),
          this,   SLOT(previewReport(int)));

  for (auto report : m_reports)
  {
    m_list->addItem(report->name());
  }
}

//------------------------------------------------------------------------
void ReportSelectorDialog::showReport()
{
  int current = m_list->currentRow();
  if (current < m_reports.size())
  {
    m_reports.at(current)->show();
  }
}

//------------------------------------------------------------------------
void ReportSelectorDialog::previewReport(int i)
{
  if (i < m_reports.size())
  {
    auto report = m_reports.at(i);

    m_title->setText(report->name());

    m_preview->setPixmap(report->preview());

    m_description->setText(report->description());
  }
}
