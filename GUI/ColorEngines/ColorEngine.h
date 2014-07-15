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

#ifndef ESPINA_COLOR_ENGINE_H
#define ESPINA_COLOR_ENGINE_H

#include "GUI/EspinaGUI_Export.h"
#include <GUI/Model/SegmentationAdapter.h>

#include <QColor>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

namespace EspINA
{
  using LUTSPtr = vtkSmartPointer<vtkLookupTable>;

  class EspinaGUI_EXPORT ColorEngine
  : public QObject
  {
    Q_OBJECT

  public:
    enum Components
    {
      None         = 0x0,
      Color        = 0x1,
      Transparency = 0x2
    };
    Q_DECLARE_FLAGS(Composition, Components)

    using LUTMap = QMap<QString, LUTSPtr>;

  public:
    virtual QColor color(SegmentationAdapterPtr seg) = 0;
    virtual LUTSPtr lut (SegmentationAdapterPtr seg) = 0;

    virtual Composition supportedComposition() const = 0;

  signals:
    void lutModified();
  };

  using ColorEngineSPtr  = std::shared_ptr<ColorEngine>;

  Q_DECLARE_OPERATORS_FOR_FLAGS(ColorEngine::Composition)

}// namespace EspINA

#endif // COLORENGINE_H
