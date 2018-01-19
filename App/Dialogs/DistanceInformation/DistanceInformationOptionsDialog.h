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
#include <ui_DistanceInformationOptionsDialog.h>
#include <GUI/Types.h>

// QT
#include <QDialog>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class CategorySelector;
    }
  }

  namespace Support
  {
    class Context;
  }

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
      struct Options
      {
          DistanceType        distanceType; /** type of distance (centroid or surface)               */
          double              minDistance;  /** minimum distance to appear in results. Zero if none. */
          double              maxDistance;  /** maximum distance to appear in results. zero if none. */
          TableType           tableType;    /** type of table to show (combined or single).          */
          CategoryAdapterSPtr category;     /** category of the segmetations to compute distance to. */
      };

      /** \brief DistanceInformationOptionsDialog class constructor.
       * \param[in] context application context.
       *
       */
      explicit DistanceInformationOptionsDialog(Support::Context &context);

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
      const double getMaximumDistance() const;

      /** \brief Returns true whether minimum distance option is enabled.
       *
       */
      bool isMinimumDistanceEnabled() const;

      /** \brief Returns the minimum distance between segmentations.
       *
       */
      const double getMinimumDistance() const;

      /** \brief Returns true whether there is a category constraint.
       *
       */
      bool isCategoryOptionEnabled() const;

      /** \brief Returns the category of the segmentations to compute distances to.
       *
       */
      const CategoryAdapterSPtr getCategory() const;

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
      /** \brief Updates the minimum value regarding the current value of the maximum.
       * \param[in] value new minimum value.
       *
       */
      void onMinimumValueChanged(double value);

      /** \brief Updates the maximum value regarding the current value of the minimum.
       * \param[in] value new minimum value.
       *
       */
      void onMaximumValueChanged(double value);

      /** \brief Updates the value of the minimum when activated regarding the current value of the maximum.
       *
       */
      void onMinimumCheckChanged(bool value);

      /** \brief Updates the value of the maximum when activated regarding the current value of the minimum.
       *
       */
      void onMaximumCheckChanged(bool value);

    private:
      GUI::Widgets::CategorySelector *m_category; /** category constraint selector. */
  };

} // namespace ESPINA

#endif // DISTANCEINFORMATIONOPTIONSDIALOG_H_
