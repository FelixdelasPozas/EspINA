/*
 * IHierarchyItem.h
 *
 *  Created on: Dec 4, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef HIERARCHYITEM_H_
#define HIERARCHYITEM_H_

#include <QObject>

class HierarchyItem
{
  public:
    enum HierarchyProperty
    {
      Empty             = 0x0,  // no hierarchy properties set
      FinalNode         = 0x1,  // node is final, meaning is solid rendered and none of it's children will be rendered
                                // by default (unless explicitly stated by the user)
      DependentNode     = 0x2,  // node rendering depends on a lower/upper associated node
      OverrideRendering = 0x3   // hierarchy rendering type overrides default rendering properties
    };
    Q_DECLARE_FLAGS(HierarchyProperties, HierarchyProperty)

    enum HierarchyRenderingType
    {
      Undefined = 0,
      Hidden,
      Opaque,
      Translucent
    };

    explicit HierarchyItem(): m_flags(Empty), m_renderingType(Opaque) {};
    virtual ~HierarchyItem() {};

    virtual void setFinalNode(bool value);
    virtual bool IsFinalNode();

    virtual void setDependentNode(bool value);
    virtual bool IsDependentNode();

    virtual void setHierarchyRenderingType(HierarchyRenderingType, bool override);
    virtual HierarchyRenderingType getHierarchyRenderingType();
    virtual bool OverridesRendering();

    virtual void resetHierarchyProperties();
  protected:
    HierarchyProperties m_flags;
    HierarchyRenderingType m_renderingType;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HierarchyItem::HierarchyProperties)

#endif /* HIERARCHYITEM_H_ */
