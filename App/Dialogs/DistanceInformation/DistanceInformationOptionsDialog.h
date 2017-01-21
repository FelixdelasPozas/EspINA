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

#ifndef DISTANCEINFORMATIONOPTIONSDIALOG_H_
#define DISTANCEINFORMATIONOPTIONSDIALOG_H_

// ESPINA
#include "DistanceInformationOptionsDialog.h"

// QT
#include <QDebug>
#include <QDialog>
#include <ui_DistanceInformationOptionsDialog.h>

// TODO add minimum and maximum distance fields
// TODO add category constraint.

namespace ESPINA
{
  /** \class DistanceInformationOptionsDialog
   * \brief Options dialog for the Distance Information Report.
   *
   */
  class DistanceInformationOptionsDialog
  : public QDialog
  , private Ui::DistanceInformationOptionsDialog
  {
      Q_OBJECT
    public:
      /** \brief Enum class containing the distance type
       *
       */
      enum class DistanceType: char { CENTROID = 0, SURFACE };

      /** \brief Emun class for the type of table.
       *
       */
      enum class TableType: char { COMBINED = 0, SINGLE };

      /** \brief Struct containing the distance information options
        *
        */
      struct Options {
          DistanceType distanceInformationType; /** type of distance (centroid or surface)      */
          double       maximumDistance;         /** maximum distance to select or zero if none. */
          TableType    tableType;               /** type of table to show (combined or single). */
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
      DistanceType getDistanceType() const;

      /** \brief Returns the type of table selected in the dialog.
       *
       */
      TableType getTableType() const;

      /** \brief Returns the options selected in the dialog.
       *
       */
      Options getOptions() const;

    private slots:
      /** \brief DistanceInformationOptionsDialog class virtual destructor.
       * \param[in] state state of the checkbox.
       *
       */
      void onMaxDistanceCheckChanged(int state);
  };

} // namespace ESPINA

#endif // DISTANCEINFORMATIONOPTIONSDIALOG_H_
