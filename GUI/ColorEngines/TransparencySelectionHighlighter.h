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


#ifndef ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H
#define ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H


#include "GUI/ColorEngines/ColorEngine.h"

#include <QMap>

namespace EspINA
{
// NOTE 2012-10-11 Consider unifying its interface with ColorEngine
class EspinaGUI_EXPORT TransparencySelectionHighlighter
{
  using LUTMap = QMap<QString, LUTSPtr>;

public:
  QColor  color(const QColor &original, bool highlight=false);
  LUTSPtr lut  (const QColor &original, bool highlight=false);

private:
  QString colorKey(const QColor &color) const;

private:
  static LUTMap m_LUT;
};

}// namespace EspINA

#endif // ESPINA_TRANSPARENCY_SELECTION_HIGHLIGHTER_H
