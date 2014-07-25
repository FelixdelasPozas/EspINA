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


#ifndef ESPINA_MULTI_COLOR_ENGINE_H
#define ESPINA_MULTI_COLOR_ENGINE_H

#include <QList>
#include "ColorEngine.h"

namespace ESPINA
{
  class EspinaGUI_EXPORT MultiColorEngine
  : public ColorEngine
  {
  public:
    virtual QColor  color(SegmentationAdapterPtr seg);
    virtual LUTSPtr lut  (SegmentationAdapterPtr seg);
    virtual ColorEngine::Composition supportedComposition() const;

    virtual void add(ColorEngineSPtr engine);
    virtual void remove(ColorEngineSPtr engine);

  protected:
    QList<ColorEngineSPtr> m_engines;
  };

  using MultiColorEngineSPtr = std::shared_ptr<MultiColorEngine>;
}// namespace ESPINA

#endif // ESPINA_MULTI_COLOR_ENGINE_H