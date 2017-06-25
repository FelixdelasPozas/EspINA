/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_ANALYZE_REPORTS_ADJACENCYMATRIXREPORT_H_
#define APP_TOOLGROUPS_ANALYZE_REPORTS_ADJACENCYMATRIXREPORT_H_

// ESPINA
#include <GUI/Types.h>
#include <Support/Report.h>

// Qt
#include <QString>

namespace ESPINA
{
  /** \class AdjacencyMatrixReport
   * \brief Adjacency matrix report.
   *
   */
  class AdjacencyMatrixReport
  : public Support::Report
  , public Support::WithContext
  {
    public:
      /** \brief AdjacencyMatrixReport class constructor.
       * \param[in] context application context
       *
       */
      explicit AdjacencyMatrixReport(Support::Context &context);

      /** \brief AdjacencyMatrixReport class virtual destructor.
       *
       */
      virtual ~AdjacencyMatrixReport()
      {}

      virtual QString name() const override;

      virtual QString description() const override;

      virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

      virtual QString requiredInputDescription() const override;

      virtual void show(SegmentationAdapterList input) const override;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_ANALYZE_REPORTS_ADJACENCYMATRIXREPORT_H_
