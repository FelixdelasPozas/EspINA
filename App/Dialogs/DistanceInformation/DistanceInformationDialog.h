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

#ifndef DISTANCEINFORMATIONDIALOG_H_
#define DISTANCEINFORMATIONDIALOG_H_

// Qt
#include <QDialog>

// ESPINA
#include "DistanceInformationOptionsDialog.h"
#include <Support/Context.h>

class QCloseEvent;

namespace ESPINA
{
  /** \class DistanceInformationDialog
    * \brief Shows the report dialog.
    *
    */
  class DistanceInformationDialog
  : public QDialog
  {
    public:
      using DistancesMap = QMap<SegmentationAdapterPtr, QMap<SegmentationAdapterPtr, double>>;

      /** \brief DistanceInformationDialog class constructor.
       * \param[in] segmentations list of segmentations of the distances or empty for all in model.
       * \param[in] distances distances map.
       * \param[in] options computation options.
       * \param[in] context application context.
       *
       */
      explicit DistanceInformationDialog(const SegmentationAdapterList segmentations,
                                         const DistancesMap distances,
                                         const DistanceInformationOptionsDialog::Options &options,
                                         Support::Context &context);

      /** \brief DistanceInformationDialog class destructor.
       *
       */
      virtual ~DistanceInformationDialog()
      {};

    protected:
      virtual void closeEvent(QCloseEvent* event);
  };

} // namespace ESPINA

#endif /* ESPINA_DISTANCEINFORMATIONDIALOG_H_ */
