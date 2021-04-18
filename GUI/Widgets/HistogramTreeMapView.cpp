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
#include <GUI/Widgets/HistogramTreeMapView.h>

// Qt
#include <QResizeEvent>
#include <QSize>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;

//--------------------------------------------------------------------
HistogramTreeMapView::HistogramTreeMapView(QWidget* parent)
: QGraphicsView{parent}
{
  setScene(new QGraphicsScene(this));
  scene()->setBackgroundBrush(QBrush{QColor::fromRgb(217, 229, 242)});

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
}

//--------------------------------------------------------------------
void HistogramTreeMapView::setHistogram(const Core::Utils::Histogram& histogram)
{
  m_histogram = histogram;

  rebuild();
}

//--------------------------------------------------------------------
const Core::Utils::Histogram& HistogramTreeMapView::histogram() const
{
  return m_histogram;
}

//--------------------------------------------------------------------
void HistogramTreeMapView::rebuild()
{
  if(!isVisible()) return;

  scene()->clear();
  scene()->invalidate(scene()->sceneRect());
  viewport()->updateGeometry();
  scene()->setSceneRect(viewport()->rect());

  if(m_histogram.isEmpty())
  {
    auto font = scene()->font();
    font.setPixelSize(22);
    font.setBold(true);
    auto item = scene()->addText("Histogram not computed.", font);
    item->setPos(scene()->width()/2 - item->boundingRect().width()/2, scene()->height()/2 - item->boundingRect().height()/2);
  }
  else
  {
    std::vector<Element> elements;
    for(int i = 0; i < 256; ++i)
    {
      struct Element element;
      element.value = i;
      element.count = m_histogram.values(i);

      elements.push_back(element);
    }
    std::sort(elements.rbegin(), elements.rend());

    QRect remainingRect = viewport()->rect();
    auto remainingCount = m_histogram.count();
    int index = 0;

    while(index < static_cast<int>(elements.size()))
    {
      auto howMany = optimalRatios(std::vector<Element>(elements.begin() + index, elements.end()), remainingRect, remainingCount);

      insertElements(std::vector<Element>(elements.begin() + index, elements.begin() + index + howMany), remainingRect, remainingCount);

      index += howMany;
    }
  }
}

//--------------------------------------------------------------------
void HistogramTreeMapView::setProgress(int progress)
{
  scene()->clear();
  scene()->invalidate(scene()->sceneRect());

  auto font = scene()->font();
  font.setPixelSize(22);
  font.setBold(true);
  auto item = scene()->addText(tr("Histogram %1% computed.").arg(progress), font);
  item->setPos(scene()->width()/2 - item->boundingRect().width()/2, scene()->height()/2 - item->boundingRect().height()/2);
}

//--------------------------------------------------------------------
void HistogramTreeMapView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);

  rebuild();
}

//--------------------------------------------------------------------
void HistogramTreeMapView::insertElements(const std::vector<Element>& elements, QRect& area, unsigned long long &remaining)
{
  if(elements.empty()) return;

  if(remaining == 0)
  {
    const auto element = elements.at(0);
    auto item = scene()->addRect(area, QPen{Qt::black}, QBrush{QColor::fromRgb(element.value, element.value, element.value)});

    QString tooltip;
    std::for_each(elements.begin(), elements.end(), [&tooltip](const Element &e) { tooltip += tr("Value %1, count %2\n").arg(e.value).arg(e.count);});
    item->setToolTip(tooltip);

    return;
  }

  unsigned long long sum = 0;
  std::for_each(elements.begin(), elements.end(), [&sum](const Element &element) { sum += element.count; });
  const double percent = std::min(1., static_cast<double>(sum)/remaining);
  remaining -= sum;

  if(sum == 0) return;

  if(area.width() < area.height())
  {
    const int height = std::round(area.height() * percent);
    QRect usedArea{area.x(), area.y(), area.width(), height};
    area.setY(area.y() + height);

    auto insertElement = [&usedArea, &area, &sum, this](const Element &element)
    {
      auto elementArea = usedArea;
      const double ratio = std::min(1., static_cast<double>(element.count)/sum);

      if(ratio == 0) return;

      auto elementWidth = std::round(area.width() * ratio);

      elementArea.setWidth(elementWidth);
      usedArea.setX(usedArea.x() + elementWidth);

      QRect intArea{static_cast<int>(std::round(elementArea.x())),static_cast<int>(std::round(elementArea.y())),
                    static_cast<int>(std::round(elementArea.width())), static_cast<int>(std::round(elementArea.height()))};

      auto item = scene()->addRect(intArea, QPen{Qt::black}, QBrush{QColor::fromRgb(element.value, element.value, element.value)});
      item->setToolTip(tr("Value %1, count %2").arg(element.value).arg(element.count));
    };

    std::for_each(elements.begin(), elements.end(), insertElement);
  }
  else
  {
    const int width = std::round(area.width() * percent);
    QRect usedArea{area.x(), area.y(), width, area.height()};
    area.setX(area.x() + width);

    auto insertElement = [&usedArea, &area, &sum, this](const Element &element)
    {
      auto elementArea = usedArea;
      const double ratio = std::min(1., static_cast<double>(element.count)/sum);

      if(ratio == 0) return;

      auto elementHeight = std::round(area.height() * ratio);

      elementArea.setHeight(elementHeight);
      usedArea.setY(usedArea.y() + elementHeight);

      QRect intArea{static_cast<int>(std::round(elementArea.x())),static_cast<int>(std::round(elementArea.y())),
                    static_cast<int>(std::round(elementArea.width())), static_cast<int>(std::round(elementArea.height()))};

      auto item = scene()->addRect(intArea, QPen{Qt::black}, QBrush{QColor::fromRgb(element.value, element.value, element.value)});
      item->setToolTip(tr("Value %1, count %2").arg(element.value).arg(element.count));
    };

    std::for_each(elements.begin(), elements.end(), insertElement);
  }
}

//--------------------------------------------------------------------
int HistogramTreeMapView::optimalRatios(const std::vector<Element> &elements, const QRect& area, const unsigned long long remaining)
{
  unsigned int result = 1;
  bool finished = false;

  if(remaining == 0) return elements.size();

  const auto primary = std::min(area.width(), area.height());
  const auto secondary = std::max(area.width(), area.height());
  double previousRatios = std::numeric_limits<double>::max();

  while(!finished && result < elements.size())
  {
    unsigned long long sum = 0;
    std::for_each(elements.begin(), elements.begin() + result, [&sum](const Element &element) { sum += element.count; });
    const double percent = std::min(1., static_cast<double>(sum)/remaining);
    auto newSecondary = secondary * percent;

    double ratios = 0;
    auto ratioOp = [&sum, &ratios, newSecondary, primary, remaining](const Element &element)
    {
      const double elementCountRatio = std::min(1., static_cast<double>(element.count)/sum);
      auto newPrimary = primary * elementCountRatio;
      const auto ratio = newPrimary / static_cast<double>(newSecondary);
      ratios += ratio;
    };
    std::for_each(elements.begin(), elements.begin() + result, ratioOp);

    ratios /= result;

    if(ratios > previousRatios || ratios < 1)
    {
      finished = true;
    }
    else
    {
      previousRatios = ratios;
      ++result;
    }
  }

  return result;
}
