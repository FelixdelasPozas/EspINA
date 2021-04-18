/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_ISSUE_LIST_DIALOG_H_
#define ESPINA_ISSUE_LIST_DIALOG_H_

// Qt
#include <QDialog>
#include "ui_ReportSelectorDialog.h"
#include <Support/Context.h>

namespace ESPINA
{
  class ReportSelectorDialog
  : public QDialog
  , public Ui::ReportSelectorDialog
  , private Support::WithContext
  {
    Q_OBJECT

  public:
    /** \brief ReportSelectorDialog class constructor.
     *
     */
    explicit ReportSelectorDialog(Support::ReportSList &reports, Support::Context &contex);

  private slots:
    void showReport();

    void previewReport(int i);

    void useSelectedSegmentationsAsReportInput(bool useSelection);

    SegmentationAdapterList reportInput();

    void setValidReport(int i, bool value);

    bool isValidReport(int i) const;

  private:
    Support::ReportSList &m_reports;
  };
} // namespace ESPINA

#endif // ESPINA_ISSUE_LIST_DIALOG_H_
