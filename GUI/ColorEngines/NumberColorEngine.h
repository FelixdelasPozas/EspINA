/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_NUMBER_COLOR_ENGINE_H
#define ESPINA_NUMBER_COLOR_ENGINE_H

// ESPINA
#include "GUI/ColorEngines/ColorEngine.h"

// Qt
#include <QMap>

namespace ESPINA
{
  class EspinaGUI_EXPORT NumberColorEngine
  : public ColorEngine
  {
  public:
  	/** \brief Implements ColorEngine::color().
  	 *
  	 */
    virtual QColor color(SegmentationAdapterPtr segmentation);

    /** \brief Implments ColorEngine::lut().
     *
     */
    virtual LUTSPtr lut(SegmentationAdapterPtr segmentation);

    /** \brief Implements ColorEngine::supportedComposition().
     *
     */
    virtual ColorEngine::Composition supportedComposition() const
    { return ColorEngine::Color; }

  private:
    LUTMap m_LUT;
  };

  using NumberColorEngineSPtr = std::shared_ptr<NumberColorEngine>;

}// namespace ESPINA

#endif // ESPINA_NUMBER_COLOR_ENGINE_H
