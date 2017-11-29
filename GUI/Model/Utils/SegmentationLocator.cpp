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
#include <GUI/Model/Utils/SegmentationLocator.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//--------------------------------------------------------------------
SegmentationLocator::SegmentationLocator(DBVHNode* dbvh)
: m_dbvh{dbvh}
{
}

//--------------------------------------------------------------------
const ViewItemAdapterSList SegmentationLocator::contains(const NmVector3& point, const NmVector3 &spacing) const
{
  return m_dbvh->contains(point, spacing);
}

//--------------------------------------------------------------------
const ViewItemAdapterSList SegmentationLocator::intersects(const Bounds& bounds, const NmVector3 &spacing) const
{
  return m_dbvh->intersects(bounds, spacing);
}

//--------------------------------------------------------------------
ManualSegmentationLocator::ManualSegmentationLocator()
: SegmentationLocator{new DBVHNode()}
{
}

//--------------------------------------------------------------------
ManualSegmentationLocator::~ManualSegmentationLocator()
{
  delete m_dbvh;
}

//--------------------------------------------------------------------
void ManualSegmentationLocator::insert(const ViewItemAdapterSList& elements)
{
  m_dbvh->insert(elements);
}

//--------------------------------------------------------------------
void ManualSegmentationLocator::remove(const ViewItemAdapterSList& elements)
{
  m_dbvh->remove(elements);
}

//--------------------------------------------------------------------
void ManualSegmentationLocator::insert(const ViewItemAdapterSPtr element)
{
  m_dbvh->insert(element);
}

//--------------------------------------------------------------------
void ManualSegmentationLocator::remove(ViewItemAdapterSPtr element)
{
  m_dbvh->remove(element);
}
