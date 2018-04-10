/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_ANALYZE_REPORTS_SPINEINFORMATIONREPORT_H_
#define APP_TOOLGROUPS_ANALYZE_REPORTS_SPINEINFORMATIONREPORT_H_

// ESPINA
#include <Support/Context.h>
#include <Support/Report.h>

namespace ESPINA
{
  /** \class SpineInformationReport
   * \brief Dendrites spine information report.
   *
   */
  class SpineInformationReport
  : public Support::Report
  , public Support::WithContext
  {
    public:
      /** \brief SpineInformationReport class constructor.
       * \param[in] context Application context.
       *
       */
      explicit SpineInformationReport(Support::Context &context);

      /** \brief SpineInformationReport class virtual destructor.
       *
       */
      virtual ~SpineInformationReport()
      {};

      virtual QString name() const override;

      virtual QString description() const override;

      virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

      virtual QString requiredInputDescription() const override;

      virtual void show(SegmentationAdapterList input) const override;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_ANALYZE_REPORTS_SPINEINFORMATIONREPORT_H_
