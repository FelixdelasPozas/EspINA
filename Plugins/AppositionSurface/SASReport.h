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

#ifndef ESPINA_SAS_REPORT_H
#define ESPINA_SAS_REPORT_H

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Support/Context.h>
#include <Support/Report.h>

namespace ESPINA
{
  class AppositionSurfacePlugin_EXPORT SASReport
  : public Support::Report
  , private Support::WithContext
  {
  public:
    explicit SASReport(Support::Context &context);

    virtual QString name() const override;

    virtual QString description() const override;

    virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const override;

    virtual QString requiredInputDescription() const override;

    virtual void show(SegmentationAdapterList input) const override;
  };
}

#endif // ESPINA_SAS_REPORT_H
