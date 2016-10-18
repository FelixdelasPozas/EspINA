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

#ifndef APP_DIALOGS_DISTANCEINFORMATION_DISTANCEINFORMATIONOPTIONSDIALOG_H_
#define APP_DIALOGS_DISTANCEINFORMATION_DISTANCEINFORMATIONOPTIONSDIALOG_H_

// ESPINA
#include "DistanceInformationOptionsDialog.h"

// QT
#include <QDebug>
#include <QDialog>
#include <ui_DistanceInformationOptionsDialog.h>

namespace ESPINA
{
  
  class DistanceInformationOptionsDialog
      : public QDialog
      , private Ui::DistanceInformationOptionsDialog
  {
      Q_OBJECT
    public:
      /** \brief Enum class containing the distance type
       *
       */
      enum class DistanceInformationType: char { CENTROID = 0, SURFACE };

      /** \brief Struct containing the distance information options
        *
        */
      struct DistanceInformationOptions {
          DistanceInformationType distanceInformationType;
          bool maximumDistanceEnabled;
          double maximumDistance;
      };

      /** \brief DistanceInformationOptionsDialog class constructor.
       * \param[in] QWidget parent widget.
       *
       */
      DistanceInformationOptionsDialog();

      /** \brief DistanceInformationOptionsDialog class virtual destructor.
       *
       */
      virtual ~DistanceInformationOptionsDialog()
      {};

      /** \brief Returns true whether centroid to centroid option is selected.
       *
       */
      bool isCentroidOptionSelected() const;

      /** \brief Returns true whether surface to surface option is selected.
       *
       */
      bool isSurfaceOptionSelected() const;

      /** \brief Returns true whether maximum distance option is enabled.
       *
       */
      bool isMaximumDistanceEnabled() const;

      /** \brief Returns the maximum distance between segmentations.
       *
       */
      double getMaximumDistance() const;

      /** \brief Returns the enum type selected in the dialog.
       *
       */
      DistanceInformationType getDistanceType() const;

      /** \brief Returns the options selected in the dialog.
       *
       */
      DistanceInformationOptions getDistanceInformationOptions() const;

    private slots:
      /** \brief DistanceInformationOptionsDialog class virtual destructor.
       *
       */
      void maximumDistanceStateChanged(int);

  };

} /* namespace ESPINA */

#endif /* APP_DIALOGS_DISTANCEINFORMATION_DISTANCEINFORMATIONOPTIONSDIALOG_H_ */
