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

#ifndef ESPINA_SUPPORT_REPORT_H
#define ESPINA_SUPPORT_REPORT_H

#include <QObject>

#include <QList>
#include <QPixmap>

#include <memory>
#include <GUI/Types.h>

class QString;

namespace ESPINA
{
  namespace Support
  {
    class Report
    : public QObject
    {
    public:
      virtual ~Report() {}

      /** \brief Return the name of the report
       *
       */
      virtual QString name() const = 0;

      /** \brief Return a long description of the report
       *
       * This description can be formated using RTF
       *
       */
      virtual QString description() const = 0;

      /** \brief Return a preview of the report
       *
       */
      virtual QPixmap preview() const
      { return QPixmap(":/espina/preview_not_available.png"); }

      virtual SegmentationAdapterList acceptedInput(SegmentationAdapterList segmentations) const = 0;

      virtual QString requiredInputDescription() const = 0;

      /** \brief Show report dialog
       *
       */
      virtual void show(SegmentationAdapterList input) const = 0;
    };
  }
}

#endif // ESPINA_SUPPORT_REPORT_H