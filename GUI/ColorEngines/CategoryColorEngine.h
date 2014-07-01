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

#ifndef ESPINA_CATEGORY_COLOR_ENGINE_H
#define ESPINA_CATEGORY_COLOR_ENGINE_H


#include "GUI/ColorEngines/ColorEngine.h"

#include <QMap>

namespace EspINA
{

class EspinaGUI_EXPORT CategoryColorEngine
: public ColorEngine
{
  Q_OBJECT
public:
  explicit CategoryColorEngine(){}

  virtual QColor  color(SegmentationAdapterPtr seg);

  virtual LUTSPtr lut  (SegmentationAdapterPtr seg);

  virtual ColorEngine::Composition supportedComposition() const
  { return ColorEngine::Color; }

protected slots:
  void updateCategoryColor(CategoryAdapterSPtr tax);

private:
  QMap<QString, LUTSPtr> m_LUT;
};

using CategoryColorEngineSPtr = std::shared_ptr<CategoryColorEngine>;

}// namespace EspINA

#endif // ESPINA_CATEGORY_COLOR_ENGINE_H
