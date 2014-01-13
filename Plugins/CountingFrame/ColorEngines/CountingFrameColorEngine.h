/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "CountingFramePlugin_Export.h"

#include <GUI/ColorEngines/ColorEngine.h>
#include <Extensions/StereologicalInclusion.h>

namespace EspINA
{
  namespace CF
  {

    class CountingFramePlugin_EXPORT CountingFrameColorEngine
    : public ColorEngine
    {

    public:
      explicit CountingFrameColorEngine();

      virtual QColor color(SegmentationAdapterPtr seg);

      virtual LUTSPtr lut(SegmentationAdapterPtr seg);

      virtual ColorEngine::Composition supportedComposition() const
      { return ColorEngine::Transparency; }

    private:
      StereologicalInclusionSPtr stereologicalInclusionExtension(SegmentationAdapterPtr segmentation);

    private:
      LUTSPtr m_excludedLUT;
      LUTSPtr m_includedLUT;
    };

  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_COLOR_ENGINE_H
