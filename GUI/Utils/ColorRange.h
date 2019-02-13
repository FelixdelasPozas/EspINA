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

#ifndef ESPINA_GUI_UTILS_COLOR_RANGE_H
#define ESPINA_GUI_UTILS_COLOR_RANGE_H

#include <GUI/EspinaGUI_Export.h>

// Qt
#include <QColor>

namespace ESPINA
{
  namespace GUI
  {
    namespace Utils
    {
      /** \class ColorRange
       * \brief Base class that defines a gradient between two given colors.
       *
       */
      class EspinaGUI_EXPORT ColorRange
      : public QObject
      {
          Q_OBJECT
        public:
          /** \brief ColorRange class constructor.
           * \param[in] min min numerical value of the gradient.
           * \param[in] max max numerical value of the gradient.
           *
           */
          explicit ColorRange(const double min, const double max);

          /** \brief ColorRange class virtual destructor.
           *
           */
          virtual ~ColorRange()
          {}

          /** \brief Returns the color that corresponds to the given value.
           * \param[in] value numerical value.
           *
           */
          QColor color(const double value) const;


          QColor color(const double value, const double min, const double max) const;

          /** \brief Sets the minimum value of the color range.
           * \param[in] value numerical value.
           *
           */
          void setMinimumValue(const double value);

          /** \brief Returns the minimum value of the color range.
           *
           */
          double minimumValue() const;

          /** \brief Sets the maximum value of the color range.
           * \param[in] value numerical value.
           *
           */
          void setMaximumValue(const double value);

          /** \brief Returns the maximum value of the color range.
           *
           */
          double maximumValue() const;

          /** \brief Sets the minimum color of the range.
           * \param[in] color QColor object.
           *
           */
          void setMinimumColor(const QColor &color);

          /** \brief Returns the minimum color of the range.
           *
           */
          QColor minimumColor() const;

          /** \brief Sets the maximum color of the color range.
           * \param[in] color QColor object.
           *
           */
          void setMaximumColor(const QColor &color);

          /** \brief Returns the maximum color of the range.
           *
           */
          QColor maximumColor() const;

          /** \brief Sets the color range to the full HUE range.
           *
           */
          void setRangeToFullHUERange();

          /** \brief Sets the color range to the lower half of the HUE range. Equivalent to truncated to 179.
           *
           */
          void setRangeToHalfHUERange();

          /** \brief Sets the range to hue from 0 to value.
           *
           */
          void setRangeToTruncatedHUERange(const int value);

        signals:
          void valueRangeChanged();
          void colorRangeChanged();

        private:
          /** \brief Helper method that computes the color of the given value in the range.
           * \param[in] value numerical value.
           * \param[in] minValue numerical range minimum value.
           * \param[in] maxValue numerical range maximum value.
           * \param[in] minColor color of the minimum range value.
           * \param[in] maxColor color of the maxumum range value.
           *
           */
          virtual QColor computeColor(const double  value,
                                      const double  minValue,
                                      const double  maxValue,
                                      const QColor &minColor,
                                      const QColor &maxColor) const = 0;

        protected:
          QColor m_minColor; /** color of the minimum range value. */
          QColor m_maxColor; /** color of the maximum range value. */
          double m_minValue; /** minimum value of the color range. */
          double m_maxValue; /** maximum value of the color range. */
      };

      /** \class RangeHSV
       * \brief ColorRange in HSV color parameters.
       *
       */
      class EspinaGUI_EXPORT RangeHSV
      : public ColorRange
      {
        public:
          /** \brief RangeHSV class constructor.
           * \param[in] min color range minimum numerical value.
           * \param[in] max color range maximum numerical value.
           *
           */
          explicit RangeHSV(const double min, const double max);

        private:
          virtual QColor computeColor(const double  value,
                                      const double  minValue,
                                      const double  maxValue,
                                      const QColor& minColor,
                                      const QColor& maxColor) const;

          /** \brief Adjusts the value in the numerical range of the color range.
           * \param[in] value numerical value.
           * \param[in] minValue minimum numerical value of the color range.
           * \param[in] maxValue maximum numerical value of the color range.
           *
           */
          double adjustRange(const double value,
                             const double minValue,
                             const double maxValue) const;

          /** \brief Returns the interpolated value inside the color range numerical value.
           * \param[in] value numerical value.
           * \param[in] minValue minimum numerical value of the color range.
           * \param[in] maxValue maximum numerical value of the color range.
           *
           */
          double interpolateFactor(const double value,
                                   const double minValue,
                                   const double maxValue) const;
      };
    }
  }
}

#endif // ESPINA_GUI_UTILS_COLOR_RANGE_H
