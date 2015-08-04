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

#include <Support/Context.h>
#include <Support/Report.h>

namespace ESPINA
{
  class SASReport
  : public Support::Report
  , private Support::WithContext
  {
  public:
    explicit SASReport(Support::Context &context);

    virtual QString name() const;

    virtual QString description() const;

    virtual QPixmap preview() const;

    virtual void show() const;
  };
}

#endif // ESPINA_SAS_REPORT_H