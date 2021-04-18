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

// Qt
#include <QReadWriteLock>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;

const QColor DEFAULT_COLOR{Qt::red};

//-----------------------------------------------------------------------------
MultiColorEngine::MultiColorEngine()
: ColorEngine("MultiColorEngine", tr("Combine multiple color engines."))
{
}

//-----------------------------------------------------------------------------
QColor MultiColorEngine::color(ConstSegmentationAdapterPtr seg)
{
  QColor color = DEFAULT_COLOR;

  {
    QReadLocker lock(&m_lock);

    switch(m_activeEngines.size())
    {
      case 0:
        break;
      case 1:
        color = m_activeEngines.first()->color(seg);
        break;
      default:
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
        break;
    }
  }

  return color;
}

//-----------------------------------------------------------------------------
LUTSPtr MultiColorEngine::lut(ConstSegmentationAdapterPtr seg)
{
  LUTSPtr lut;

  {
    QReadLocker lock(&m_lock);

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
  }

  return lut;
}

//-----------------------------------------------------------------------------
ColorEngine::Composition MultiColorEngine::supportedComposition() const
{
  ColorEngine::Composition composition = None;

  {
    QReadLocker lock(&m_lock);

    for(auto engine: m_activeEngines)
    {
      composition |= engine->supportedComposition();
    }
  }

  return composition;
}

//-----------------------------------------------------------------------------
void MultiColorEngine::add(ColorEngineSPtr engine)
{
  auto updated = false;

  {
    QWriteLocker lock(&m_lock);

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

        updated = true;
      }
    }
  }

  if(updated) emit modified();
}

//-----------------------------------------------------------------------------
void MultiColorEngine::remove(ColorEngineSPtr engine)
{
  bool updated = false;

  {
    QWriteLocker lock(&m_lock);

    if(m_availableEngines.contains(engine))
    {
      m_availableEngines.removeOne(engine);

      if(engine->isActive())
      {
        m_activeEngines.removeOne(engine.get());
      }

      disconnect(engine.get(), SIGNAL(activated(bool)),
                 this,         SLOT(onColorEngineActivated(bool)));

      disconnect(engine.get(), SIGNAL(modified()),
                 this,         SIGNAL(modified()));

      if (engine->isActive()) updated = true;
    }
  }

  if(updated) emit modified();
}

//-----------------------------------------------------------------------------
void MultiColorEngine::onColorEngineActivated(bool active)
{
  auto engine = dynamic_cast<ColorEngine*>(sender());

  {
    QWriteLocker lock(&m_lock);
    if (active && !m_activeEngines.contains(engine))
    {
      m_activeEngines << engine;
    }
    else
    {
      if(m_activeEngines.contains(engine))
      {
        m_activeEngines.removeOne(engine);
      }
    }
  }

  emit modified();
}

//-----------------------------------------------------------------------------
const QList<ColorEngineSPtr> ESPINA::GUI::ColorEngines::MultiColorEngine::activeEngines() const
{
  QList<ColorEngineSPtr> engines;

  {
    QReadLocker lock(&m_lock);
    for(auto engine: m_availableEngines)
    {
      if(m_activeEngines.contains(engine.get()))
      {
        engines << engine;
      }
    }
  }

  return engines;
}

//-----------------------------------------------------------------------------
const QList<ColorEngineSPtr> ESPINA::GUI::ColorEngines::MultiColorEngine::availableEngines() const
{
  QReadLocker lock(&m_lock);

  return m_availableEngines;
}

//-----------------------------------------------------------------------------
const ColorEngineSPtr ESPINA::GUI::ColorEngines::MultiColorEngine::getEngine(const QString& engineId)
{
  ColorEngineSPtr engine = nullptr;

  {
    QReadLocker lock(&m_lock);
    for(auto registeredEngine: m_availableEngines)
    {
      if(registeredEngine->id() == engineId)
      {
        return registeredEngine;
      }
    }
  }

  return engine;
}
