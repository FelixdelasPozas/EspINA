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

#ifndef ESPINA_ANALYZE_TOOLGROUP_H
#define ESPINA_ANALYZE_TOOLGROUP_H

// ESPINA
#include <ToolGroups/ToolGroup.h>
#include <Support/Context.h>
#include <Support/Report.h>

namespace ESPINA
{
  class ReportsTool;

  /** \class AnalyzeToolGroup
   * \brief Implemens the analize toolgroup.
   *
   */
  class AnalyzeToolGroup
  : public ToolGroup
  {
      Q_OBJECT
    public:
      /** \brief AnalyzeToolGroup class constructor.
       * \param[in] context application context.
       * \param[in] parent QWidget parent of this one.
       *
       */
      explicit AnalyzeToolGroup(Support::Context &context, QWidget *parent = nullptr);

      /** \brief MeasuresTools class destructor.
       *
       */
      virtual ~AnalyzeToolGroup();

      /** \brief Registers a report in the toolgroup.
       * \param[in] report report smart pointer.
       *
       */
      void registerReport(Support::ReportSPtr report);

    private:
      std::shared_ptr<ReportsTool> m_reports; /** reports tool. */
  };

} /* namespace ESPINA */

#endif // ESPINA_ANALYZE_TOOLGROUP_H
