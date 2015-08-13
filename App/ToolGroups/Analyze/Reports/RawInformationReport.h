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

#ifndef ESPINA_RAW_INFORMATION_REPORT_H
#define ESPINA_RAW_INFORMATION_REPORT_H

#include <Support/Report.h>
#include <Support/Context.h>

namespace ESPINA
{
  class RawInformationReport
  : public Support::Report
  , private Support::WithContext
  {
  public:
    explicit RawInformationReport(Support::Context &context);

    virtual QString name() const override;

    virtual QString description() const override;

    virtual QPixmap preview() const override;

    virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

    virtual QString requiredInputDescription() const override;

    virtual void show(SegmentationAdapterList input) const override;
  };
}

#endif // ESPINA_RAW_INFORMATION_REPORT_H
