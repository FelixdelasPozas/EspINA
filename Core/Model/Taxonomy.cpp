#include "Core/Model/Taxonomy.h"

#include <QFile>
#include <QPixmap>

#include <iostream>
#include <QDebug>
#include <assert.h>

//#include <assert.h>
/*
#define ASSERT(x) \
  if (! (x)) \
  { \
    std::cout << "ERROR: Assert " << #x << " failed\n"; \
    std::cout << " in " << __FILE__ << ":" << __LINE__  << "\n"; \
  } \
  assert(x)
*/

using namespace EspINA;

const QString TaxonomyElement::X_DIM = "Dim_X";
const QString TaxonomyElement::Y_DIM = "Dim_Y";
const QString TaxonomyElement::Z_DIM = "Dim_Z";

//------------------------------------------------------------------------
TaxonomyElement::TaxonomyElement(TaxonomyElementPtr parent,
                                 const QString &name,
                                 const QString &RGBColor)
: m_parent(parent)
, m_name(name)
, m_color(RGBColor)
{
}

//------------------------------------------------------------------------
TaxonomyElement::~TaxonomyElement()
{
  qDebug() << "Destroy node " << m_name;
}

//------------------------------------------------------------------------
void TaxonomyElement::setName(const QString &name)
{
  Q_ASSERT(element(name).isNull());
  m_name = name;
}

//------------------------------------------------------------------------
QString TaxonomyElement::name() const
{
  return m_name;
}

//------------------------------------------------------------------------
QString TaxonomyElement::qualifiedName() const
{
  if (m_parent && !m_parent->name().isEmpty())
    return m_parent->qualifiedName() + "/" + m_name;
  else
    return m_name;
}


//------------------------------------------------------------------------
SharedTaxonomyElementPtr TaxonomyElement::createElement(const QString& name)
{
  Q_ASSERT(element(name).isNull());

  SharedTaxonomyElementPtr element(new TaxonomyElement(this, name));
  element->setColor(m_color);

  m_elements << element;

  return element;
}

//------------------------------------------------------------------------
void TaxonomyElement::deleteElement(TaxonomyElementPtr element)
{
  SharedTaxonomyElementPtr subNode;

  int index = 0;
  while (subNode.isNull() && index < m_elements.size())
  {
    if (m_elements[index].data() == element)
      subNode = m_elements[index];
    else
      index++;
  }

  Q_ASSERT(!subNode.isNull());
  m_elements.removeAt(index);
}

//------------------------------------------------------------------------
SharedTaxonomyElementPtr TaxonomyElement::element(const QString& name)
{
  SharedTaxonomyElementPtr res;

  int i = 0;
  while (res.isNull() && i < m_elements.size())
  {
    if (m_elements[i]->name() == name)
      res = m_elements[i];
    i++;
  }

  return res;
}

//------------------------------------------------------------------------
void TaxonomyElement::addProperty(const QString& prop, const QVariant& value)
{
  m_properties[prop] = value;
}

//------------------------------------------------------------------------
void TaxonomyElement::removeProperty(const QString& prop)
{
  if (m_properties.contains(prop))
    m_properties.remove(prop);
}

//------------------------------------------------------------------------
QVariant TaxonomyElement::property(const QString& prop) const
{
  return m_properties.value(prop,QVariant());
}


//------------------------------------------------------------------------
void TaxonomyElement::print(int level)
{
  if (!m_name.isEmpty())
  {
    std::cout <<
    std::string(level*2, ' ') << m_name.toStdString() << std::endl;
    if (m_properties.size() > 0)
    {
      std::cout << std::string(level*2, ' ') << "<" << std::endl;
      foreach(QString key, m_properties.keys())
      {
        std::cout << std::string((level+1)*2, ' ')
        << key.toStdString() << ": " << m_properties[key].toString().toStdString()
        << std::endl;
      }
      std::cout << std::string(level*2, ' ') << ">" << std::endl;
    }
  }

  foreach (SharedTaxonomyElementPtr node, m_elements)
    node->print(level+1);
}

//------------------------------------------------------------------------
QVariant TaxonomyElement::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      return name();
    case Qt::DecorationRole:
    {
      QPixmap icon(16,16);
      icon.fill(color());
      return icon;
    }
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool TaxonomyElement::setData(const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
    setName(value.toString());
    return true;
  }
  if (role == Qt::DecorationRole)
  {
    setColor(value.value<QColor>());
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
TaxonomyElementPtr EspINA::taxonomyElementPtr(ModelItemPtr item)
{
  Q_ASSERT(EspINA::TAXONOMY == item->type());
  TaxonomyElementPtr ptr = dynamic_cast<TaxonomyElementPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//-----------------------------------------------------------------------------
SharedTaxonomyElementPtr taxonomyElementPtr(SharedModelItemPtr& item)
{
  Q_ASSERT(EspINA::TAXONOMY == item->type());
  SharedTaxonomyElementPtr ptr = qSharedPointerDynamicCast<TaxonomyElement>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}


//-----------------------------------------------------------------------------
// TAXONOMY
//-----------------------------------------------------------------------------
const QString Taxonomy::ROOT = QString();

Taxonomy::Taxonomy()
: m_root(new TaxonomyElement(NULL, ROOT))
{
}

//-----------------------------------------------------------------------------
Taxonomy::~Taxonomy()
{
  qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
SharedTaxonomyElementPtr Taxonomy::createElement(const QString& name,
                                                 TaxonomyElementPtr parent)
{
  TaxonomyElementPtr parentNode = parent;

  if (!parentNode)
    parentNode = m_root.data();

  return parentNode->createElement(name);
}

//-----------------------------------------------------------------------------
SharedTaxonomyElementPtr Taxonomy::createElement(const QString& name,
                                                 SharedTaxonomyElementPtr parent)
{
  return createElement(name, parent.data());
}


//-----------------------------------------------------------------------------
void Taxonomy::deleteElement(TaxonomyElementPtr element)
{
  Q_ASSERT(element);

  if (element != m_root.data())
  {
    TaxonomyElementPtr parentElement = element->parent();
    parentElement->deleteElement(element);
  }
  else
    m_root.clear();
}

//-----------------------------------------------------------------------------
void Taxonomy::deleteElement(SharedTaxonomyElementPtr element)
{
  deleteElement(element.data());
}


//-----------------------------------------------------------------------------
SharedTaxonomyElementPtr Taxonomy::element(const QString& qualifiedName)
{
  QStringList path = qualifiedName.split("/", QString::SkipEmptyParts);
  SharedTaxonomyElementPtr node = m_root;
  for(int i = 0; i < path.length(); i++)
  {
    node = node->element(path[i]);
    Q_ASSERT(!node.isNull());
  }

  return node;
  // MEGA TODO 2012-12-15
//   bool exits = false;
//   foreach(TaxonomyElementPtr sibling, m_parent->m_elements)
//   {
//     exits = exits || sibling->name() == name;
//   }
//   if (!exits)
//     m_name = name;
//   QString::SectionFlag flag = QString::SectionSkipEmpty;
//   return m_root->element(qualifiedName);
// 
//   QString rootName = qualifiedName.section("/",0,0,flag);
// 
//   if (rootName != m_root->name())
//     return NULL;
// 
//   QString subName = qualifiedName.section("/",1,-1,flag);
//   if (subName.isEmpty())
//     return m_root;
//   else
//     return m_root->element(subName);
}

//-----------------------------------------------------------------------------
SharedTaxonomyElementPtr Taxonomy::parent(const SharedTaxonomyElementPtr element) const
{
  QStringList path = element->qualifiedName().split("/", QString::SkipEmptyParts);

  SharedTaxonomyElementPtr parent = m_root;
  for(int i = 0; i < path.length() - 1; i++)
  {
    parent = parent->element(path[i]);
    Q_ASSERT(!parent.isNull());
  }

  return parent;
}

//-----------------------------------------------------------------------------
void Taxonomy::print(int indent)
{
  m_root->print(indent);
}



