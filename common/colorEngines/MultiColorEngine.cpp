/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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

//-----------------------------------------------------------------------------
QColor MultiColorEngine::color(const Segmentation* seg)
{
  if (m_engines.isEmpty())
    return QColor(Qt::red);

  int r=0, g=0, b=0, a=0;

  for(int i=0; i<m_engines.size(); i++)
  {
    QColor c = m_engines[i]->color(seg);
    r += c.red();
    g += c.green();
    b += c.blue();
    a += c.alpha();
  }

  r /= m_engines.size();
  g /= m_engines.size();
  b /= m_engines.size();
  a /= m_engines.size();

  return QColor(r,g,b,a);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> MultiColorEngine::lut(const Segmentation* seg)
{
  if (m_engines.size() == 1)
    return m_engines.first()->lut(seg);

  double alpha = 0.8;
  QColor c = color(seg);
  vtkSmartPointer<vtkLookupTable> seg_lut = vtkLookupTable::New();
  seg_lut->Allocate();
  seg_lut->SetNumberOfTableValues(2);
  seg_lut->Build();
  seg_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  seg_lut->SetTableValue(1, c.redF(), c.greenF(), c.blueF(), alpha);
  seg_lut->Modified();

  return seg_lut;
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