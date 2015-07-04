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

// ESPINA
#include "MultiColorEngine.h"

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

//-----------------------------------------------------------------------------
MultiColorEngine::MultiColorEngine()
: ColorEngine("MultiColorEngine", tr("Combine multiple color engines"))
{
}

//-----------------------------------------------------------------------------
QColor MultiColorEngine::color(SegmentationAdapterPtr seg)
{
  QColor color(Qt::red);

  if (m_activeEngines.size() == 1)
  {
    color = m_activeEngines.first()->color(seg);
  }
  else if (m_activeEngines.size() > 1)
  {
    int r=0, g=0, b=0, a=0;
    int rgbComponents=0, alphaComponents=0;

    for(auto engine: m_activeEngines)
    {
      auto c = engine->color(seg);
      if (engine->supportedComposition().testFlag(Color))
      {
        r += c.red();
        g += c.green();
        b += c.blue();
        rgbComponents++;
      }

      if (engine->supportedComposition().testFlag(Transparency))
      {
        a += c.alpha();
        alphaComponents++;
      }
    }

    if (rgbComponents > 0)
    {
      r /= rgbComponents;
      g /= rgbComponents;
      b /= rgbComponents;
    }

    if (alphaComponents > 0)
    {
      a /= alphaComponents;
    }
    else
    {
      // Prevent transparent color if no engine supports deals transparency
      a = 255;
    }

    color = QColor(r,g,b,a);
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr MultiColorEngine::lut(SegmentationAdapterPtr seg)
{
  LUTSPtr lut;

  if (m_activeEngines.size() == 1)
  {
    lut = m_activeEngines.first()->lut(seg);
  }
  else
  {
    auto alpha = 0.8;
    auto c     = color(seg);

    lut = LUTSPtr::New();
    lut->Allocate();
    lut->SetNumberOfTableValues(2);
    lut->Build();
    lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
    lut->Modified();
  }

  return lut;
}

//-----------------------------------------------------------------------------
ColorEngine::Composition MultiColorEngine::supportedComposition() const
{
  ColorEngine::Composition composition = None;

  for(auto engine: m_activeEngines)
  {
    composition |= engine->supportedComposition();
  }

  return composition;
}

//-----------------------------------------------------------------------------
void MultiColorEngine::add(ColorEngineSPtr engine)
{
  if (!m_availableEngines.contains(engine))
  {
    m_availableEngines << engine;

    connect(engine.get(), SIGNAL(activated(bool)),
            this,         SLOT(onColorEngineActivated(bool)));

    connect(engine.get(), SIGNAL(modified()),
            this,         SIGNAL(modified()));

    if (engine->isActive() && !m_activeEngines.contains(engine.get()))
    {
      m_activeEngines << engine.get();

      emit modified();
    }
  }
}

//-----------------------------------------------------------------------------
void MultiColorEngine::remove(ColorEngineSPtr engine)
{
  m_availableEngines.removeOne(engine);
  m_activeEngines   .removeOne(engine.get());

  disconnect(engine.get(), SIGNAL(activated(bool)),
             this,         SLOT(onColorEngineActivated(bool)));

  disconnect(engine.get(), SIGNAL(modified()),
             this,         SIGNAL(modified()));

  if (engine->isActive())
  {
    emit modified();
  }
}

//-----------------------------------------------------------------------------
void MultiColorEngine::onColorEngineActivated(bool active)
{
  auto engine = dynamic_cast<ColorEngine*>(sender());

  if (active)
  {
    m_activeEngines << engine;
  }
  else
  {
    m_activeEngines.removeOne(engine);
  }

  emit modified();
}
