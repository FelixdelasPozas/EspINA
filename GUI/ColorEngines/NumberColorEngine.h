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

#include "GUI/ColorEngines/ColorEngine.h"

#include <QMap>

namespace EspINA
{
  class EspinaGUI_EXPORT NumberColorEngine
  : public ColorEngine
  {
  public:
    virtual QColor color(SegmentationAdapterPtr segmentation);

    virtual LUTSPtr lut(SegmentationAdapterPtr segmentation);

    virtual ColorEngine::Composition supportedComposition() const
    { return ColorEngine::Color; }

  private:
    QMap<QString, LUTSPtr> m_LUT;
  };

  using NumberColorEngineSPtr = std::shared_ptr<NumberColorEngine>;

}// namespace EspINA

#endif // ESPINA_NUMBER_COLOR_ENGINE_H
