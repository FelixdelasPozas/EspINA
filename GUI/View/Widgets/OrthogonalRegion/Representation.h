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

#ifndef ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H
#define ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H

#include <Core/Utils/Bounds.h>

#include <QObject>
#include <QColor>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class Representation
          : public QObject
          {
            Q_OBJECT
          public:
            explicit Representation();

            explicit Representation(const NmVector3 &resolution, const Bounds &bounds);

            void setResolution(const NmVector3 &resolution);

            NmVector3 resolution() const;

            void setBounds(const Bounds &bounds);

            Bounds bounds() const;

            /** \brief Sets the color of the representation.
             * \param[in] color pointer to a vector of three double corresponding to the r,g,b values.
             *
             */
            void setColor(const QColor &color);

            QColor representationColor() const;

            /** \brief Sets the representation pattern.
             * \param[in] pattern pattern in hexadecimal.
             */
            void setRepresentationPattern(int pattern);

            int representationPattern() const;

          signals:
            void resolutionChanged(const NmVector3 &resolution);

            void boundsChanged(const Bounds &bounds);

            void colorChanged(const QColor &color);

            void patternChanged(const int pattern);

          private:
            NmVector3 m_resolution;
            Bounds    m_bounds;
            QColor    m_color;
            int       m_pattern;
          };
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H
