/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_COUNTING_FRAME_COLOR_ENGINE_H
#define ESPINA_COUNTING_FRAME_COLOR_ENGINE_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include <Extensions/StereologicalInclusion.h>

// ESPINA
#include <GUI/ColorEngines/ColorEngine.h>

namespace ESPINA
{
  namespace CF
  {
    /** \class ColorEngine
     * \brief Implements the coloring engine for the plugin.
     *
     */
    class CountingFramePlugin_EXPORT ColorEngine
    : public GUI::ColorEngines::ColorEngine
    {
        Q_OBJECT
      public:
        /** \brief ColorEngine class constructor.
         *
         */
        explicit ColorEngine();

        virtual QColor color(ConstSegmentationAdapterPtr seg);

        virtual LUTSPtr lut(ConstSegmentationAdapterPtr seg);

        virtual ColorEngine::Composition supportedComposition() const
        { return ColorEngine::Transparency; }

        /** \brief Returns the opacity for excluded segmentations
         *
         *  Opactity range is [0, 1]
         */
        double exlcusionOpacity() const;

        virtual GUI::ColorEngines::ColorEngineSPtr clone()
        { return std::make_shared<ColorEngine>(); }

      public slots:
        /** \brief Sets the opacity for excluded segmentations
         *
         *  \param value for the exclusion opacity in the range [0, 1]
         */
        void setExclusionOpacity(const double value);

      private:
        double  m_exclusionOpacity; /** opacity of the excluded segmentations.  */
        LUTSPtr m_excludedLUT;      /** color table for excluded segmentations. */
        LUTSPtr m_includedLUT;      /** color table for included segmentations. */
    };

    using CountingFrameColorEngineSPtr = std::shared_ptr<ColorEngine>;
  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_COLOR_ENGINE_H
