/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "SkeletonToolsUtils.h"
#include <GUI/View/View2D.h>
#include <ToolGroups/Segment/Skeleton/SkeletonToolWidget2D.h>

using namespace ESPINA;
using namespace ESPINA::SkeletonToolsUtils;
using namespace ESPINA::GUI::Representations::Settings;

//--------------------------------------------------------------------
SkeletonToolWidget2D::SkeletonToolWidget2D(SkeletonToolsEventHandlerSPtr handler, SkeletonPoolSettingsSPtr settings)
: SkeletonWidget2D{handler, settings}
, m_toolHandler   {handler}
, m_settings      {settings}
{
  connect(handler.get(), SIGNAL(signalConnection(const QString, const int, const Plane)),
          this,          SLOT(onConnectionSignaled(const QString, const int, const Plane)), Qt::DirectConnection);

  connect(handler.get(), SIGNAL(changeStrokeTo(const QString, const int, const Plane)),
          this,          SLOT(onStrokeChangeSignaled(const QString, const int, const Plane)), Qt::DirectConnection);
}

//--------------------------------------------------------------------
SkeletonToolWidget2D::~SkeletonToolWidget2D()
{
  disconnect(m_toolHandler.get(), SIGNAL(signalConnection(const QString, const int, const Plane)),
             this,                SLOT(onConnectionSignaled(const QString, const int, const Plane)));

  disconnect(m_toolHandler.get(), SIGNAL(changeStrokeTo(const QString, const int, const Plane)),
             this,                SLOT(onStrokeChangeSignaled(const QString, const int, const Plane)));
}
//--------------------------------------------------------------------
void SkeletonToolWidget2D::onConnectionSignaled(const QString &category, const int strokeIndex, const Plane plane)
{
  if(!m_widget || !m_view) return;
  auto view2d = view2D_cast(m_view);
  if(!view2d || view2d->plane() != plane) return;

  auto &strokes = STROKES[category];
  auto index = std::min(std::max(0, strokeIndex), strokes.size() -1);

  m_widget->createConnection(strokes.at(index));
}

//--------------------------------------------------------------------
void SkeletonToolWidget2D::onStrokeChangeSignaled(const QString &category, const int strokeIndex, const Plane plane)
{
  if(!m_widget || !m_view) return;
  auto view2d = view2D_cast(m_view);
  if(!view2d || view2d->plane() != plane) return;

  auto &strokes = STROKES[category];
  auto index = std::min(std::max(0, strokeIndex), strokes.size() -1);

  m_widget->changeStroke(strokes.at(index));
}

//--------------------------------------------------------------------
bool SkeletonToolWidget2D::isStartNode(const NmVector3 &point)
{
  if(m_widget) return m_widget->isStartNode(point);

  return false;
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr SkeletonToolWidget2D::cloneImplementation()
{
  return std::make_shared<SkeletonToolWidget2D>(m_toolHandler, m_settings);
}
