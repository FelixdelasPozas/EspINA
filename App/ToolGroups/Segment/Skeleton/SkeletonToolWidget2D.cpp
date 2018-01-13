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

//--------------------------------------------------------------------
SkeletonToolWidget2D::SkeletonToolWidget2D(SkeletonToolsEventHandlerSPtr handler)
: SkeletonWidget2D{handler}
, m_toolHandler   {handler}
{
  connect(handler.get(), SIGNAL(signalConnection(const QString, const int, const Plane)),
          this,          SLOT(onConnectionSignaled(const QString, const int, const Plane)), Qt::DirectConnection);

  connect(handler.get(), SIGNAL(changeStrokeTo(const QString, const int, const Plane)),
          this,          SLOT(onStrokeChangeSignaled(const QString, const int, const Plane)), Qt::DirectConnection);
}

//--------------------------------------------------------------------
void SkeletonToolWidget2D::onConnectionSignaled(const QString &category, const int strokeIndex, const Plane plane)
{
  if(!m_widget || !m_view) return;
  auto view2d = view2D_cast(m_view);
  if(!view2d || view2d->plane() != plane) return;

  m_widget->createConnection(STROKES[category].at(strokeIndex));
}

//--------------------------------------------------------------------
void SkeletonToolWidget2D::onStrokeChangeSignaled(const QString &category, const int strokeIndex, const Plane plane)
{
  if(!m_widget || !m_view) return;
  auto view2d = view2D_cast(m_view);
  if(!view2d || view2d->plane() != plane) return;

  m_widget->changeStroke(STROKES[category].at(strokeIndex));
}

//--------------------------------------------------------------------
bool SkeletonToolWidget2D::isStartNode(const NmVector3 &point)
{
  if(m_widget) return m_widget->isStartNode(point);

  return false;
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr ESPINA::SkeletonToolWidget2D::cloneImplementation()
{
  return std::make_shared<SkeletonToolWidget2D>(m_toolHandler);
}
