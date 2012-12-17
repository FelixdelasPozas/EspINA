/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#include "MultiColorEngine.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
QColor MultiColorEngine::color(SegmentationPtr seg)
{
  if (m_engines.isEmpty())
    return QColor(Qt::red);

  if (m_engines.size() == 1)
    return m_engines[0]->color(seg);

  int r=0, g=0, b=0, a=0;
  int rgbComponents=0, alphaComponents=0;

  for(int i=0; i<m_engines.size(); i++)
  {
    QColor c = m_engines[i]->color(seg);
    if (m_engines[i]->supportedComposition().testFlag(Color))
    {
      r += c.red();
      g += c.green();
      b += c.blue();
      rgbComponents++;
    }
    if (m_engines[i]->supportedComposition().testFlag(Transparency))
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
    a /= alphaComponents;
  else // Prevent transparent color if no engine supports deals transparency
    a = 255;

  return QColor(r,g,b,a);
}

//-----------------------------------------------------------------------------
LUTPtr MultiColorEngine::lut(SegmentationPtr seg)
{
  if (m_engines.size() == 1)
    return m_engines.first()->lut(seg);

  double alpha = 0.8;
  QColor c = color(seg);
  LUTPtr seg_lut = LUTPtr::New();
  seg_lut->Allocate();
  seg_lut->SetNumberOfTableValues(2);
  seg_lut->Build();
  seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
  seg_lut->Modified();

  return seg_lut;
}

//-----------------------------------------------------------------------------
ColorEngine::Composition MultiColorEngine::supportedComposition() const
{
  ColorEngine::Composition composition = None;

  foreach(ColorEnginePtr engine, m_engines)
    composition |= engine->supportedComposition();

  return composition;
}

//-----------------------------------------------------------------------------
void MultiColorEngine::addColorEngine(ColorEnginePtr engine)
{
  m_engines << engine;
}

//-----------------------------------------------------------------------------
void MultiColorEngine::removeColorEngine(ColorEnginePtr engine)
{
  m_engines.removeAll(engine);
}