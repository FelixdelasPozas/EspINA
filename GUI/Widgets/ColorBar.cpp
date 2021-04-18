/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ColorBar.h"
#include <GUI/Utils/ColorRange.h>

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QBrush>

using namespace ESPINA;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::Widgets;

//------------------------------------------------------------------------
ColorBar::ColorBar(ColorRange *range, QWidget *parent)
: QWidget(parent)
, m_colorRange(range)
{
  connect(m_colorRange, SIGNAL(valueRangeChanged()),
          this,         SLOT(updateTooltip()));

  connect(m_colorRange, SIGNAL(colorRangeChanged()),
          this,         SLOT(repaint()));

  updateTooltip();
}

//------------------------------------------------------------------------
void ColorBar::paintEvent(QPaintEvent *event)
{
  const int BORDER_WIDTH = 1;

  QRect rect(0, 0, width(), height());

  int wi = rect.width()  - 2*BORDER_WIDTH;
  int hi = rect.height() - 2*BORDER_WIDTH;

  if (m_colorMap.height() != hi || m_colorMap.width() != wi)
  {
    auto img = rangeImage(m_colorRange, wi, hi);

    m_colorMap.convertFromImage(img);
  }

  QPainter p(this);

  p.setBrush(QBrush(Qt::black));
  p.drawRect(rect);
  p.drawPixmap(BORDER_WIDTH, BORDER_WIDTH, m_colorMap);
}

//------------------------------------------------------------------------
void ColorBar::updateTooltip()
{
  auto toolTip = QString("(%1, %2)").arg(m_colorRange->minimumValue())
                                    .arg(m_colorRange->maximumValue());
  setToolTip(toolTip);
}

//------------------------------------------------------------------------
QImage ColorBar::rangeImage(const Utils::ColorRange *range, int width, int height)
{
  QImage img(width, height, QImage::Format_RGB32);

  for (int x = 0; x < width; x++)
  {
    auto color = range->color(x, 0, width);
    for (int y = 0; y < height; y++)
    {
      img.setPixel(x, y, color.rgb());
    }
  }

  return img;
}
