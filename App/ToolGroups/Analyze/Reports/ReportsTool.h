/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_REPORTS_TOOL_H
#define ESPINA_REPORTS_TOOL_H

#include <Support/Widgets/ProgressTool.h>

#include <Support/Report.h>

namespace ESPINA
{
  /** \brief ReportsTool
   * \brief Simple tool for showing the reports dialog.
   *
   */
  class ReportsTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT

    public:
      /** \brief Reports tool class constructor.
       * \param[in] context application context.
       *
       */
      explicit ReportsTool(Support::Context &context);

      /** \brief ReportsTool class virtual destructor.
       *
       */
      virtual ~ReportsTool();

      /** \brief Registers a new report into the reports dialog.
       * \param[in] report Report object.
       *
       */
      void registerReport(Support::ReportSPtr report);

    private slots:
      /** \brief Shows the report dialog when the button is clicked.
       * \param[in] unused
       *
       */
      void onTriggered(bool unused);

    private:
      Support::ReportSList m_reports; /** list of reports to show in the report dialog. */
    };
}

#endif // ESPINA_REPORTS_TOOL_H
