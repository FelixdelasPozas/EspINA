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
TaxonomyElement::TaxonomyElement(const QString &name,
                                 const QString &RGBColor)
: m_parent(NULL)
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
TaxonomyElementPtr TaxonomyElement::createElement(const QString& name)
{
  Q_ASSERT(element(name).isNull());

  TaxonomyElementPtr element(new TaxonomyElement(name));
  element->setColor(m_color);

  m_elements << element;

  return element;
}

//------------------------------------------------------------------------
void TaxonomyElement::deleteElement(TaxonomyElementPtr element)
{
  Q_ASSERT(m_elements.contains(element));

  m_elements.removeOne(element);
}

//------------------------------------------------------------------------
TaxonomyElementPtr TaxonomyElement::element(const QString& name)
{
  TaxonomyElementPtr res;

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

  foreach (TaxonomyElementPtr node, m_elements)
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
TaxonomyElementPtr EspINA::taxonomyElementPtr(ModelItemPtr &item)
{
  Q_ASSERT(EspINA::TAXONOMY == item->type());
  TaxonomyElementPtr ptr = qSharedPointerDynamicCast<TaxonomyElement>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}


//-----------------------------------------------------------------------------
// TAXONOMY
//-----------------------------------------------------------------------------
const QString Taxonomy::ROOT = QString();

Taxonomy::Taxonomy()
: m_root(new TaxonomyElement(ROOT))
{
}

//-----------------------------------------------------------------------------
Taxonomy::~Taxonomy()
{
  qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
TaxonomyElementPtr Taxonomy::createElement(const QString& name, TaxonomyElementPtr parent)
{
  TaxonomyElementPtr parentNode = parent;

  if (parentNode.isNull())
    parentNode = m_root;

  return parentNode->createElement(name);
}

//-----------------------------------------------------------------------------
void Taxonomy::deleteElement(TaxonomyElementPtr element)
{
  Q_ASSERT(!element.isNull());

  TaxonomyElementPtr parentElement = parent(element);
  parentElement->deleteElement(element);
}

//-----------------------------------------------------------------------------
TaxonomyElementPtr Taxonomy::element(const QString& qualifiedName)
{
  // MEGA TODO 2012-12-15
  Q_ASSERT(false);
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
TaxonomyElementPtr Taxonomy::parent(const TaxonomyElementPtr element) const
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void Taxonomy::print(int indent)
{
  m_root->print(indent);
}



