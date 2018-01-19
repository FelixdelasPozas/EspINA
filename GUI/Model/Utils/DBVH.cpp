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
#include <GUI/Model/Utils/DBVH.h>
#include <QFile>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//--------------------------------------------------------------------
DBVHNode::DBVHNode(DBVHNode *parent, const unsigned int depth, const ViewItemAdapterSList &elements)
: m_parent {parent}
, m_left   {nullptr}
, m_right  {nullptr}
, m_element{nullptr}
, m_depth  {depth}
, m_size   {elements.size()}
{
  switch(m_size)
  {
    case 0:
      break;
    case 1:
      m_element = elements.first();
      m_bounds = m_element->output()->bounds();
      connect(m_element.get(), SIGNAL(outputModified()), this, SLOT(refit()));
      break;
    default:
      m_bounds = Utils::AABB<ViewItemAdapterSPtr>(elements);
      auto eList = elements;
      eList.detach();
      qSort(eList.begin(), eList.end(), splitMethod<ViewItemAdapterSPtr>(bestSplitAxis()));
      auto center = eList.size() / 2;
      m_left = std::make_shared<DBVHNode>(this, m_depth + 1, eList.mid(0, center));
      m_right = std::make_shared<DBVHNode>(this, m_depth + 1, eList.mid(center));
      break;
  }
}

//--------------------------------------------------------------------
DBVHNode::~DBVHNode()
{
  clear();
}

//--------------------------------------------------------------------
void DBVHNode::debugInformation(QFile *file) const
{
  QString result;

  for(unsigned int i = 0; i < m_depth; ++i) result += " ";

  auto text = result + QString("Node: ") + (isRoot() ? QString("R"):QString("")) + (isLeaf() ? QString("L"):QString("")) + QString(" depth %1 size %2 bounds").arg(m_depth).arg(m_size) + m_bounds.toString();

  if(file && file->isOpen())
  {
    file->write(text.toAscii());
  }
  else
  {
    qDebug() << text;
  }

  if(!isLeaf())
  {
    m_left->debugInformation(file);
    m_right->debugInformation(file);
  }
}

//--------------------------------------------------------------------
const ViewItemAdapterSList DBVHNode::elements() const
{
  ViewItemAdapterSList result;

  if(isLeaf())
  {
    result << m_element;
  }
  else
  {
    result << m_left->elements() << m_right->elements();
  }

  return result;
}

//--------------------------------------------------------------------
void DBVHNode::refit(bool propagate)
{
  auto oldBounds = m_bounds;

  if(isLeaf())
  {
    m_bounds = m_element->output()->bounds();
  }
  else
  {
    m_bounds = boundingBox(m_left->AABB(), m_right->AABB());
  }

  if(oldBounds.areValid() && (oldBounds != m_bounds))
  {
    optimize();
  }

  if(propagate && !isRoot() && (oldBounds != m_bounds))
  {
    m_parent->refit(true);
  }
}

//--------------------------------------------------------------------
void DBVHNode::refit()
{
  refit(true);
}

//--------------------------------------------------------------------
void DBVHNode::clear()
{
  if(m_element)
  {
    disconnect(m_element.get(), SIGNAL(outputModified()), this, SLOT(refit()));
  }

  m_left    = nullptr;
  m_right   = nullptr;
  m_element = nullptr;
  m_bounds  = Bounds();
  m_size    = 0;
}

//--------------------------------------------------------------------
void DBVHNode::insert(const ViewItemAdapterSList &elements)
{
  for(auto element: elements)
  {
    insert(element);
  }
}

//--------------------------------------------------------------------
void DBVHNode::remove(const ViewItemAdapterSList &elements)
{
  for(auto element: elements)
  {
    remove(element);
  }
}

//--------------------------------------------------------------------
void DBVHNode::insert(const ViewItemAdapterSPtr element)
{
  if(!element) return;

  auto eBounds = element->output()->bounds();
  ++m_size;

  if(!m_element && !m_left && !m_right)
  {
    m_element = element;
    m_bounds  = eBounds;
    connect(m_element.get(), SIGNAL(outputModified()), this, SLOT(refit()));
    return;
  }

  if(m_element)
  {
    m_bounds = boundingBox(eBounds, m_bounds);
    ViewItemAdapterSList left, right;
    left  << (splitMethod<ViewItemAdapterSPtr>(bestSplitAxis())(m_element, element) ? m_element : element);
    right << (splitMethod<ViewItemAdapterSPtr>(bestSplitAxis())(m_element, element) ? element : m_element);
    m_left    = std::make_shared<DBVHNode>(this, m_depth + 1, left);
    m_right   = std::make_shared<DBVHNode>(this, m_depth + 1, right);
    disconnect(m_element.get(), SIGNAL(outputModified()), this, SLOT(refit()));
    m_element = nullptr;
  }
  else
  {
    if(surfaceArea(boundingBox(m_left->AABB(), eBounds)) < surfaceArea(boundingBox(m_right->AABB(), eBounds)))
    {
      m_left->insert(element);
    }
    else
    {
      m_right->insert(element);
    }

    m_bounds = boundingBox(m_left->AABB(), m_right->AABB());
  }
}

//--------------------------------------------------------------------
bool DBVHNode::remove(ViewItemAdapterSPtr element)
{
  if(m_element)
  {
    if(m_element == element)
    {
      clear();
      return true;
    }
  }
  else
  {
    auto eBounds = element->output()->bounds();
    for(auto branch: { m_left, m_right })
    {
      if(ESPINA::contains(branch->AABB(),eBounds) && branch->remove(element))
      {
        --m_size;

        if(branch->size() == 0)
        {
          rebuild(); // will delete the empty node.
        }
        else
        {
          refit(false);
        }

        return true;
      }
    }
  }

  return false;
}

//--------------------------------------------------------------------
void DBVHNode::rebuild()
{
  if(isLeaf()) return;

  ViewItemAdapterSList elements;
  if(m_left && m_left->size() > 0)   elements << m_left->elements();
  if(m_right && m_right->size() > 0) elements << m_right->elements();

  clear();

  m_size = elements.size();

  if(m_size == 1)
  {
    m_element = elements.first();
    m_bounds = m_element->output()->bounds();
    m_left = nullptr;
    m_right = nullptr;
  }
  else
  {
    m_bounds = Utils::AABB<ViewItemAdapterSPtr>(elements);
    qSort(elements.begin(), elements.end(), splitMethod<ViewItemAdapterSPtr>(bestSplitAxis()));
    auto center = m_size / 2;
    m_left  = std::make_shared<DBVHNode>(this, m_depth + 1, elements.mid(0,center));
    m_right = std::make_shared<DBVHNode>(this, m_depth + 1, elements.mid(center));
  }
}

//--------------------------------------------------------------------
void DBVHNode::optimize()
{
  if(m_element || (m_left->isLeaf() && m_right->isLeaf())) return;

  const auto sahNode = surfaceArea(m_left->AABB()) + surfaceArea(m_right->AABB());
  Rotation rot{Rotation::NONE};
  double sah = std::numeric_limits<double>::max();

  for(auto rotation: {Rotation::L_RL, Rotation::L_RR, Rotation::R_LL, Rotation::R_LR, Rotation::LL_RR, Rotation::LL_RL})
  {
    auto sahOptm = evaluate(rotation);
    if(sah > sahOptm)
    {
      rot = rotation;
      sah = sahOptm;
    }
  }

  if(rot != Rotation::NONE)
  {
    // check if worth the cost
    if( ((sahNode - sah)/sahNode) >= 0.3 )
    {
      rotate(rot);
    }
  }

  m_left->optimize();
  m_right->optimize();
}

//--------------------------------------------------------------------
const ESPINA::Axis DBVHNode::bestSplitAxis() const
{
  if(!m_bounds.areValid()) return Axis::X;

  double length[3];
  for(unsigned int i: {0,1,2}) length[i] = m_bounds.lenght(toAxis(i));

  if((length[0] > length[1]) && (length[0] > length[2])) return Axis::X;
  if(length[1] > length[2])  return Axis::Y;
  return Axis::Z;
}

//--------------------------------------------------------------------
const double DBVHNode::evaluate(const Rotation rot)
{
  double sah = std::numeric_limits<double>::max();

  switch(rot)
  {
    case Rotation::NONE:
      sah = surfaceArea(m_left->AABB()) + surfaceArea(m_right->AABB());
      break;
    case Rotation::L_RL:
      if(!m_right->isLeaf()) sah = surfaceArea(m_right->m_left->AABB()) + surfaceArea(boundingBox(m_left->AABB(), m_right->m_right->AABB()));
      break;
    case Rotation::L_RR:
      if(!m_right->isLeaf()) sah = surfaceArea(m_right->m_right->AABB()) + surfaceArea(boundingBox(m_left->AABB(), m_right->m_left->AABB()));
      break;
    case Rotation::R_LL:
      if(!m_left->isLeaf()) sah = surfaceArea(m_left->m_left->AABB()) + surfaceArea(boundingBox(m_right->AABB(), m_left->m_right->AABB()));
      break;
    case Rotation::R_LR:
      if(!m_left->isLeaf()) sah = surfaceArea(m_left->m_right->AABB()) + surfaceArea(boundingBox(m_right->AABB(), m_left->m_left->AABB()));
      break;
    case Rotation::LL_RR:
      if(!m_left->isLeaf() && !m_right->isLeaf()) sah = surfaceArea(boundingBox(m_right->m_right->AABB(), m_left->m_right->AABB())) + surfaceArea(boundingBox(m_right->m_left->AABB(), m_left->m_left->AABB()));
      break;
    case Rotation::LL_RL:
      if(!m_left->isLeaf() && !m_right->isLeaf()) sah = surfaceArea(boundingBox(m_right->m_left->AABB(), m_left->m_right->AABB())) + surfaceArea(boundingBox(m_left->m_left->AABB(), m_right->m_right->AABB()));
      break;
    default:
      break;
  }

  return sah;
}

//--------------------------------------------------------------------
void DBVHNode::rotate(const Rotation rot)
{
  auto updateSize = [](std::shared_ptr<DBVHNode> left, std::shared_ptr<DBVHNode> right)
  {
    left->m_size = left->elements().size();
    right->m_size = right->elements().size();
    if(left->m_parent) left->m_parent->m_size = left->size() + right->size();
  };

  switch(rot)
  {
    case Rotation::NONE:
      break;
    case Rotation::L_RL:
      {
        auto me = m_left->m_parent;
        auto swapNode = m_left;
        m_left = m_right->m_left;
        m_left->m_parent = me;
        m_right->m_left = swapNode;
        m_right->m_left->m_parent = m_right.get();
        updateSize(m_left, m_right);
        m_right->refit(false);
      }
      break;
    case Rotation::L_RR:
      {
        auto me = m_left->m_parent;
        auto swapNode = m_left;
        m_left = m_right->m_right;
        m_left->m_parent = me;
        m_right->m_right = swapNode;
        m_right->m_right->m_parent = m_right.get();
        updateSize(m_left, m_right);
        m_right->refit(false);
      }
      break;
    case Rotation::R_LL:
      {
        auto me = m_right->m_parent;
        auto swapNode = m_right;
        m_right = m_left->m_left;
        m_right->m_parent = me;
        m_left->m_left = swapNode;
        m_left->m_left->m_parent = m_left.get();
        updateSize(m_left, m_right);
        m_left->refit(false);
      }
      break;
    case Rotation::R_LR:
      {
        auto me = m_right->m_parent;
        auto swapNode = m_right;
        m_right = m_left->m_right;
        m_right->m_parent = me;
        m_left->m_right = swapNode;
        m_left->m_right->m_parent = m_left.get();
        updateSize(m_left, m_right);
        m_left->refit(false);
      }
      break;
    case Rotation::LL_RR:
      {
        auto swapNode = m_left->m_left;
        m_left->m_left = m_right->m_right;
        m_right->m_right = swapNode;
        m_left->m_left->m_parent = m_left.get();
        m_right->m_right->m_parent = m_right.get();
        updateSize(m_left, m_right);
        m_left->refit(false);
        m_right->refit(false);
      }
      break;
    case Rotation::LL_RL:
      {
        auto swapNode = m_left->m_left;
        m_left->m_left = m_right->m_left;
        m_right->m_left = swapNode;
        m_left->m_left->m_parent = m_left.get();
        m_right->m_left->m_parent = m_right.get();
        updateSize(m_left, m_right);
        m_left->refit(false);
        m_right->refit(false);
      }
      break;
    default:
      break;
  }
}

//--------------------------------------------------------------------
const ViewItemAdapterSList DBVHNode::contains(const NmVector3& point, const NmVector3 &spacing) const
{
  ViewItemAdapterSList result;

  if(ESPINA::contains(m_bounds, point, spacing))
  {
    if(m_element)
    {
      auto sSpacing = m_element->output()->spacing();
      if(ESPINA::contains(m_bounds, point, sSpacing))
      {
        result << m_element;
      }
    }
    else
    {
      result << m_left->contains(point, spacing) + m_right->contains(point, spacing);
    }
  }

  return result;
}

//--------------------------------------------------------------------
const ViewItemAdapterSList DBVHNode::intersects(const Bounds& bounds, const NmVector3 &spacing) const
{
  ViewItemAdapterSList result;

  if(intersect(m_bounds, bounds, spacing))
  {
    if(m_element)
    {
      auto sSpacing = m_element->output()->spacing();
      if(intersect(m_bounds, bounds, sSpacing))
      {
        result << m_element;
      }
    }
    else
    {
      result << m_left->intersects(bounds, spacing) + m_right->intersects(bounds, spacing);
    }
  }

  return result;
}
