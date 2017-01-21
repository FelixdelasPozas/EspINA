/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#ifndef DISTANCEINFORMATIONREPORT_H_
#define DISTANCEINFORMATIONREPORT_H_

#include <Support/Report.h>
#include <Support/Context.h>

namespace ESPINA
{
  /** \class DistanceInformationReport
   * \brief Distance between segmentations report.
   *
   */
  class DistanceInformationReport
  : public Support::Report
  , private Support::WithContext
  {
    public:
      /** \brief DistanceInformationReport class constructor.
       * \param[in] context application context.
       *
       */
      explicit DistanceInformationReport(Support::Context &context);

      /** \brief DistanceInformationReport class virtual destructor.
       *
       */
      virtual ~DistanceInformationReport()
      {};

      virtual QString name() const override;

      virtual QString description() const override;

      virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

      virtual QString requiredInputDescription() const override;

      virtual void show(SegmentationAdapterList input) const override;
  };
} // namespace ESPINA

#endif // DISTANCEINFORMATIONREPORT_H_
