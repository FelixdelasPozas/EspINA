/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <GUI/Widgets/HistogramBarsView.h>

// C++
#include <cmath>

// Qt
#include <QResizeEvent>
#include <QSize>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
HistogramBarsView::HistogramBarsView(QWidget* parent)
: QGraphicsView{parent}
, m_normalTransform{1,0,0,0,1,0,0,0,1}
, m_inverseYTransform{1,0,0,0,-1,0,0,0,1}
{
  setScene(new QGraphicsScene(this));
  scene()->setBackgroundBrush(QBrush{QColor::fromRgb(217, 229, 242)});

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
}

//--------------------------------------------------------------------
void HistogramBarsView::setHistogram(const Core::Utils::Histogram& histogram)
{
  m_histogram = histogram;

  rebuild();
}

//--------------------------------------------------------------------
const Core::Utils::Histogram& HistogramBarsView::histogram() const
{
  return m_histogram;
}

//--------------------------------------------------------------------
void HistogramBarsView::rebuild()
{
  if(!isVisible()) return;

  scene()->clear();
  scene()->invalidate(scene()->sceneRect());
  viewport()->updateGeometry();
  scene()->setSceneRect(viewport()->rect());

  if(m_histogram.isEmpty())
  {
    if(transform() != m_normalTransform) setTransform(m_normalTransform);

    auto font = scene()->font();
    font.setPixelSize(22);
    font.setBold(true);
    auto item = scene()->addText("Histogram not computed.", font);
    item->setPos(scene()->width()/2 - item->boundingRect().width()/2, scene()->height()/2 - item->boundingRect().height()/2);
  }
  else
  {
    if(transform() != m_inverseYTransform) setTransform(m_inverseYTransform);

    unsigned long long max = 0;
    for(int i = 0; i < 256; ++i) max = std::max(max, m_histogram.values(i));
    double barWidth  = scene()->width() / 256.;
    double barHeight = scene()->height();

    for(int i = 0; i < 256; ++i)
    {
      QRect rect{static_cast<int>(barWidth*i), 0, static_cast<int>(std::round(barWidth))+1, static_cast<int>(std::round(barHeight))};
      auto count = m_histogram.values(i);
      new HistogramBar(this, rect, static_cast<double>(count)/max, i, count);
    }
  }
}

//--------------------------------------------------------------------
void HistogramBarsView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);

  rebuild();
}

//--------------------------------------------------------------------
void HistogramBarsView::setProgress(int progress)
{
  scene()->clear();
  scene()->invalidate(scene()->sceneRect());

  if(transform() != m_normalTransform) setTransform(m_normalTransform);

  auto font = scene()->font();
  font.setPixelSize(22);
  font.setBold(true);
  auto item = scene()->addText(tr("Histogram %1% computed.").arg(progress), font);
  item->setPos(scene()->width()/2 - item->boundingRect().width()/2, scene()->height()/2 - item->boundingRect().height()/2);
}

//--------------------------------------------------------------------
HistogramBar::HistogramBar(HistogramBarsView * parent, const QRectF &rect, double height, unsigned char number, unsigned long long count)
: QGraphicsRectItem{rect}
{
  Q_ASSERT(parent);

  // no pen to make the background invisible, then we create another rect to express the value.
  // This way the tooltip appears even if the mouse is above the bar.
  setPen(Qt::NoPen);
  setBrush(Qt::NoBrush);


  QRectF barRect = rect;
  barRect.setHeight(rect.height() * height);

  const auto color = QColor::fromRgb(number, number, number);
  auto filledRect = new QGraphicsRectItem(barRect, this);
  filledRect->setPen(QPen{color});
  filledRect->setBrush(QBrush{color});
  filledRect->update();

  parent->scene()->addItem(this);

  auto tooltip = QObject::tr("Value %1, count %2").arg(number).arg(count);
  setToolTip(tooltip);
  filledRect->setToolTip(tooltip);

  setZValue(1);
  filledRect->setZValue(1);
}
