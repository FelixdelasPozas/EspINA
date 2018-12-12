/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ColorEngines/ConnectionsColorEngine.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::ColorEngines;

//--------------------------------------------------------------------
ConnectionsColorEngine::ConnectionsColorEngine()
: ColorEngine     {"ConnectionsColorEngine", tr("Colors segmentations according to a connection criteria.")}
, m_validHUE      {QColor{Qt::green}.hue()}
, m_invalidHUE    {QColor{Qt::red}.hue()}
, m_incompleteHUE {QColor{Qt::blue}.hue()}
, m_unconnectedHUE{QColor{Qt::yellow}.hue()}
{
}

//--------------------------------------------------------------------
QColor ConnectionsColorEngine::color(ConstSegmentationAdapterPtr segmentation)
{
  auto smartPtr    = segmentation->model()->smartPointer(const_cast<SegmentationAdapter *>(segmentation));
  auto connections = segmentation->model()->connections(smartPtr);

  if(connections.isEmpty())                                           return QColor::fromHsv(m_unconnectedHUE, 255, 255);
  if(connections.size() < m_criteria.size())                          return QColor::fromHsv(m_incompleteHUE, 255, 255);
  if(connections.size() == m_criteria.size() && isValid(connections)) return QColor::fromHsv(m_validHUE, 255, 255);

  return QColor::fromHsv(m_invalidHUE, 255, 255);
}

//--------------------------------------------------------------------
LUTSPtr ConnectionsColorEngine::lut(ConstSegmentationAdapterPtr segmentation)
{
  auto  segColor = color(segmentation);
  auto  segLUT   = LUTSPtr::New();

  segLUT->Allocate();
  segLUT->SetNumberOfTableValues(2);
  segLUT->Build();
  segLUT->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  segLUT->SetTableValue(1, segColor.redF(), segColor.greenF(), segColor.blueF(), 1.0);
  segLUT->Modified();

  return segLUT;
}

//--------------------------------------------------------------------
void ConnectionsColorEngine::setCriteriaInformation(const QStringList& criteria, int valid,
                                                    int invalid, int incomplete, int unconnected)
{
  if(criteria != m_criteria || m_validHUE != valid || m_invalidHUE != invalid || m_incompleteHUE != incomplete || m_unconnectedHUE != unconnected)
  {
    m_criteria = criteria;
    m_validHUE = valid;
    m_invalidHUE = invalid;
    m_incompleteHUE = incomplete;
    m_unconnectedHUE = unconnected;

    emit modified();
  }
}

//--------------------------------------------------------------------
const bool ConnectionsColorEngine::isValid(const ConnectionList &connections) const
{
  auto current = m_criteria;
  for (auto connection : connections)
  {
    if (current.isEmpty()) return false;

    auto category = connection.item2->category()->classificationName();
    if (current.contains(category))
    {
      current.removeOne(category);
    }
    else
    {
      for (auto criteriaCategory : current)
      {
        if (category.startsWith(criteriaCategory, Qt::CaseInsensitive))
        {
          current.removeOne(criteriaCategory);
          break;
        }
      }
    }
  }

  if (current.isEmpty()) return true;

  return false;
}
