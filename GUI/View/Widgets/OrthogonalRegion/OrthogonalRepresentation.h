/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H
#define ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Utils/Bounds.h>

// Qt
#include <QObject>
#include <QColor>

// C++
#include <memory>

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
          /** \class OrthogonalRepresentation
           * \brief Implements an orthogonal region in 3D.
           *
           */
          class EspinaGUI_EXPORT OrthogonalRepresentation
          : public QObject
          {
            Q_OBJECT
          public:
            /** interaction mode. */
            enum class Mode { FIXED, RESIZABLE };

          public:
            /** \brief OrthogonalRepresentation class empty constructor.
             *
             */
            explicit OrthogonalRepresentation();

            /** \brief OrthogonalRepresentation class constructor.
             * \param[in] resolution Voxel resolution in 3D.
             * \param[in] bounds Region bounds.
             *
             */
            explicit OrthogonalRepresentation(const NmVector3 &resolution, const Bounds &bounds);

            /** \brief Sets the interaction mode of the region.
             * \param[in] mode Mode enum value.
             *
             */
            void setMode(const Mode mode);

            /** \brief Returns the current interaction mode.
             *
             */
            const Mode mode() const;

            /** \brief Sets the voxel resolution of the region.
             * \param[in] resolution Vector of voxel resolution in Nm.
             *
             *
             */
            void setResolution(const NmVector3 &resolution);

            /** \brief Returns the voxel resolution of the region.
             *
             */
            const NmVector3 resolution() const;

            /** \brief Sets the bounds of the region.
             * \param[in] bounds Bounds object reference.
             *
             */
            void setBounds(const Bounds &bounds);

            /** \brief Returns the bounds of the region.
             *
             */
            const Bounds bounds() const;

            /** \brief Sets the color of the representation.
             * \param[in] color pointer to a vector of three double corresponding to the r,g,b values.
             *
             */
            void setColor(const QColor &color);

            /** \brief Returns the color of the representation for the region.
             *
             */
            const QColor representationColor() const;

            /** \brief Sets the representation pattern.
             * \param[in] pattern pattern in hexadecimal.
             *
             */
            void setRepresentationPattern(const int pattern);

            /** \brief Returns the patter of the representation for the region.
             *
             */
            const int representationPattern() const;

          signals:
            void modeChanged(const OrthogonalRepresentation::Mode mode);

            void resolutionChanged(const NmVector3 &resolution);

            void boundsChanged(const Bounds &bounds);

            void colorChanged(const QColor &color);

            void patternChanged(const int pattern);

          private:
            Mode      m_mode;       /** current interaction mode of the region.       */
            NmVector3 m_resolution; /** voxel resolution of the region in Nm.         */
            Bounds    m_bounds;     /** bounds of the region.                         */
            QColor    m_color;      /** color of the representation for the region.   */
            int       m_pattern;    /** pattern of the representation for the region. */
          };

          using OrthogonalRepresentationSPtr = std::shared_ptr<OrthogonalRepresentation>;
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_ORTHOGONALREGION_REPRESENTATION_H
