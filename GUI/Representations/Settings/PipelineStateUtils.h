/*

    Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_PIPELINE_STATE_UTILS_H_
#define ESPINA_PIPELINE_STATE_UTILS_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Types.h>
#include <GUI/Representations/RepresentationState.h>

// Qt
#include <QColor>

namespace ESPINA
{
  namespace Representations
  {
    const QString CROSSHAIR_X = "CHX";
    const QString CROSSHAIR_Y = "CHY";
    const QString CROSSHAIR_Z = "CHZ";

    const QString VISIBLE     = "Visible";

    const QString OPACITY     = "Opacity";
    const QString CONTRAST    = "Contrast";
    const QString BRIGHTNESS  = "Brightness";
    const QString STAIN       = "Stain";
    const QString COLOR       = "Color";
    const QString DEPTH       = "Depth";

    const QString TIME_STAMP  = "TimeStamp";
  }

  RepresentationState EspinaGUI_EXPORT channelPipelineSettings(ConstChannelAdapterPtr channel);

  RepresentationState EspinaGUI_EXPORT segmentationPipelineSettings(ConstSegmentationAdapterPtr segmentation);

  double EspinaGUI_EXPORT brightness(const RepresentationState &state);

  double EspinaGUI_EXPORT contrast(const RepresentationState &state);

  double EspinaGUI_EXPORT opacity(const RepresentationState &state);

  QColor EspinaGUI_EXPORT stain(const RepresentationState &state);
}

#endif // ESPINA_PIPELINE_STATE_UTILS_H_
