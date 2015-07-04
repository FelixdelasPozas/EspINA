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

#include <QColor>

namespace ESPINA {
  namespace GUI {
    namespace Utils {

      class ColorRange
      : public QObject
      {
        Q_OBJECT
      public:
        explicit ColorRange(const double min, const double max);

        virtual ~ColorRange() {}

        QColor color(const double value) const;

        QColor color(const double value, const double min, const double max);

        void setMinimumValue(const double value);

        double minimumValue() const;

        void setMaximumValue(const double value);

        double maximumValue() const;

        void setMinimumColor(const QColor &color);

        QColor minimumColor() const;

        void setMaximumColor(const QColor &color);

        QColor maximumColor() const;

      signals:
        void valueRangeChanged();
        void colorRangeChanged();

      private:
        virtual QColor computeColor(const double  value,
                                    const double  minValue,
                                    const double  maxValue,
                                    const QColor &minColor,
                                    const QColor &maxColor) const = 0;

      private:
        QColor m_minColor;
        QColor m_maxColor;
        double m_minValue;
        double m_maxValue;
      };

      class RangeHSV
      : public ColorRange
      {
      public:
        explicit RangeHSV(const double min, const double max);

      private:
        virtual QColor computeColor(const double  value,
                                    const double  minValue,
                                    const double  maxValue,
                                    const QColor& minColor,
                                    const QColor& maxColor) const;

        double adjustRange(const double value,
                           const double minValue,
                           const double maxValue) const;

        double interpolateFactor(const double value,
                                 const double minValue,
                                 const double maxValue) const;
      };
    }
  }
}

#endif // ESPINA_GUI_UTILS_COLOR_RANGE_H
