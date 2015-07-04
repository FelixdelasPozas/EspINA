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
ColorBar::~ColorBar()
{
}

//------------------------------------------------------------------------
void ColorBar::paintEvent(QPaintEvent *event)
{
  const int BORDER_WITH = 1;

  QRect rect(0, 0, width(), height());

  int wi = rect.width()  - 2*BORDER_WITH;
  int hi = rect.height() - 2*BORDER_WITH;

  if (m_colorMap.height() != hi || m_colorMap.width() != wi)
  {
    QImage img(wi, hi, QImage::Format_RGB32);

    for (int x = 0; x < wi; x++)
    {
      auto color = m_colorRange->color(x, 0, wi);
      for (int y = 0; y < hi; y++)
      {
        img.setPixel(x, y, color.rgb());
      }
    }

    m_colorMap.convertFromImage(img);
  }

  QPainter p(this);

  p.setBrush(QBrush(Qt::black));
  p.drawRect(rect);
  p.drawPixmap(BORDER_WITH, BORDER_WITH, m_colorMap);
}

//------------------------------------------------------------------------
void ColorBar::updateTooltip()
{
  auto toolTip = QString("(%1, %2)").arg(m_colorRange->minimumValue())
                                    .arg(m_colorRange->maximumValue());
  setToolTip(toolTip);
}
