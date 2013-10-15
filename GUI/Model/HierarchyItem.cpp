/*
 * IHierarchyItem.cpp
 *
 *  Created on: Dec 4, 2012
 *      Author: Félix de las Pozas Álvarez
 */


#include "HierarchyItem.h"
#include <QDebug>

//-----------------------------------------------------------------------------
void HierarchyItem::setFinalNode(bool value)
{
  if (value)
    m_flags |= FinalNode;
  else
    m_flags &= ~FinalNode;
}

//-----------------------------------------------------------------------------
bool HierarchyItem::IsFinalNode()
{
  return m_flags.testFlag(FinalNode);
}

//-----------------------------------------------------------------------------
void HierarchyItem::setDependentNode(bool value)
{
  if (value)
    m_flags |= DependentNode;
  else
    m_flags &= ~DependentNode;
}

//-----------------------------------------------------------------------------
bool HierarchyItem::IsDependentNode()
{
  return m_flags.testFlag(DependentNode);
}

//-----------------------------------------------------------------------------
void HierarchyItem::setHierarchyRenderingType(HierarchyRenderingType type, bool override)
{
  m_renderingType = type;

  if (override)
    m_flags |= OverrideRendering;
  else
    m_flags &= ~OverrideRendering;
}

//-----------------------------------------------------------------------------
HierarchyItem::HierarchyRenderingType HierarchyItem::getHierarchyRenderingType()
{
  return m_renderingType;
}

//-----------------------------------------------------------------------------
bool HierarchyItem::OverridesRendering()
{
  return m_flags.testFlag(OverrideRendering);
}

//-----------------------------------------------------------------------------
void HierarchyItem::resetHierarchyProperties()
{
  m_flags = 0x0;
}

