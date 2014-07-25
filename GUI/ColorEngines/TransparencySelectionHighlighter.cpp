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

#include "TransparencySelectionHighlighter.h"

#include <QDebug>

using namespace ESPINA;

ColorEngine::LUTMap TransparencySelectionHighlighter::m_LUT;

//-----------------------------------------------------------------------------
QColor TransparencySelectionHighlighter::color(const QColor& original,
                                               bool highlight)
{
  auto suggestedColor = original;

  if (highlight)
    suggestedColor.setAlphaF(suggestedColor.alphaF()*1.0);
  else
    suggestedColor.setAlphaF(suggestedColor.alphaF()*0.6);

  return suggestedColor;
}

//-----------------------------------------------------------------------------
LUTSPtr TransparencySelectionHighlighter::lut(const QColor& original,
                                             bool highlight)
{
  auto segColor = color(original, highlight);
  auto key = colorKey(segColor);

  if (!m_LUT.contains(key))
  {
//     qDebug() << "Generating LUT for:" << key;
    double rgba[4] =
    {
      segColor.redF(),
      segColor.greenF(),
      segColor.blueF(),
      segColor.alphaF()
    };

    auto segLUT = LUTSPtr::New();
    segLUT->Allocate();
    segLUT->SetNumberOfTableValues(2);
    segLUT->Build();
    segLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    segLUT->SetTableValue(1, rgba);
    segLUT->Modified();

    m_LUT.insert(key, segLUT);
  }

  return m_LUT[key];
}

//-----------------------------------------------------------------------------
QString TransparencySelectionHighlighter::colorKey(const QColor& color) const
{
  return QString().sprintf("%03d%03d%03d%03d",
                          color.red(),
                          color.green(),
                          color.blue(),
                          color.alpha());
}
