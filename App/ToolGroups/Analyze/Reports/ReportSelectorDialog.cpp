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
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Dialogs/DefaultDialogs.h>

using ESPINA::GUI::DefaultDialogs;

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
ReportSelectorDialog::ReportSelectorDialog(Support::ReportSList &reports, Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget())
, WithContext(context)
, m_reports(reports)
{
  setupUi(this);
  setWindowTitle(tr("Report Selector"));

  connect(this, SIGNAL(accepted()),
          this, SLOT(showReport()));

  connect(m_list, SIGNAL(currentRowChanged(int)),
          this,   SLOT(previewReport(int)));

  connect(m_selectionReport, SIGNAL(toggled(bool)),
          this,              SLOT(useSelectedSegmentationsAsReportInput(bool)));

  for (auto report : m_reports)
  {
    m_list->addItem(report->name());
  }

  m_list->setCurrentRow(0);

  useSelectedSegmentationsAsReportInput(false);
}

//------------------------------------------------------------------------
void ReportSelectorDialog::showReport()
{
  int current = m_list->currentRow();
  if (current < m_reports.size())
  {
    auto &report = m_reports.at(current);

    report->show(report->acceptedInput(reportInput()));
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

    if (isValidReport(i))
    {
      m_description->setText(report->description());
    }
    else
    {
      m_description->setText(report->requiredInputDescription());
    }
  }
}

//------------------------------------------------------------------------
void ReportSelectorDialog::useSelectedSegmentationsAsReportInput(bool useSelection)
{
  auto input = reportInput();

  for (int i = 0; i < m_reports.size(); ++i)
  {
    bool validReport = !m_reports[i]->acceptedInput(input).isEmpty();

    setValidReport(i, validReport);
  }

  previewReport(m_list->currentRow());
}


//----------------------------------------------------------------------------
SegmentationAdapterList ReportSelectorDialog::reportInput()
{
  return m_selectionReport->isChecked()?getSelectedSegmentations()
                                       :toRawList<SegmentationAdapter>(getModel()->segmentations());
}

//----------------------------------------------------------------------------
void ReportSelectorDialog::setValidReport(int i, bool value)
{
  m_list->item(i)->setForeground(value?Qt::black:Qt::gray);
}

//----------------------------------------------------------------------------
bool ReportSelectorDialog::isValidReport(int i) const
{
  return m_list->item(i)->foreground() == Qt::black;
}
