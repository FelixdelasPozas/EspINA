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
   //qDebug() << "Destroy node " << m_name;
}

//------------------------------------------------------------------------
void TaxonomyElement::setName(const QString &name)
{
  Q_ASSERT(element(name).get() == NULL);
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
void TaxonomyElement::setColor(const QColor &color)
{
  if (m_color != color)
  {
    m_color = color;

    emit colorChanged(this);
  }
}

//------------------------------------------------------------------------
TaxonomyElementSPtr TaxonomyElement::createElement(const QString& name)
{
  Q_ASSERT(!element(name));

  TaxonomyElementSPtr taxElement(new TaxonomyElement(this, name));
  taxElement->setColor(m_color);

  m_elements << taxElement;

  return taxElement;
}

//-----------------------------------------------------------------------------
void TaxonomyElement::addElement(TaxonomyElementSPtr taxElement)
{
  Q_ASSERT(!m_elements.contains(taxElement));

  taxElement->m_parent = this;

  m_elements << taxElement;
}

//------------------------------------------------------------------------
void TaxonomyElement::deleteElement(TaxonomyElementPtr taxElement)
{
  TaxonomyElementSPtr subNode;

  int index = 0;
  while (!subNode && index < m_elements.size())
  {
    if (m_elements[index].get() == taxElement)
      subNode = m_elements[index];
    else
      index++;
  }

  if (subNode)
    m_elements.removeAt(index);
}

//------------------------------------------------------------------------
TaxonomyElementSPtr TaxonomyElement::element(const QString& name)
{
  TaxonomyElementSPtr res;

  int i = 0;
  while (!res && i < m_elements.size())
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

  foreach (TaxonomyElementSPtr node, m_elements)
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
    case TypeRole:
      return EspINA::TAXONOMY;
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
TaxonomyElementSPtr taxonomyElementPtr(ModelItemSPtr& item)
{
  Q_ASSERT(EspINA::TAXONOMY == item->type());
  TaxonomyElementSPtr ptr = boost::dynamic_pointer_cast<TaxonomyElement>(item);
  Q_ASSERT(ptr != NULL);

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
   //qDebug() << "Destroy taxonomy";
}

//-----------------------------------------------------------------------------
TaxonomyElementSPtr Taxonomy::createElement(const QString&     qualifiedName,
                                            TaxonomyElementPtr parent)
{
  TaxonomyElementPtr parentNode = parent;

  if (!parentNode)
    parentNode = m_root.get();

  TaxonomyElementSPtr requestedTaxonomy;

  if (!qualifiedName.isEmpty())
  {
    QStringList path = qualifiedName.split("/", QString::SkipEmptyParts);
    for (int i = 0; i < path.size(); ++i)
    {
      requestedTaxonomy = parentNode->element(path.at(i));
      if (!requestedTaxonomy)
      {
        requestedTaxonomy = parentNode->createElement(path.at(i));
      }
      parentNode = requestedTaxonomy.get();
    }
  }
  else
  {
    requestedTaxonomy = parentNode->element(QString("Unspecified"));

    if (!requestedTaxonomy)
      requestedTaxonomy = parentNode->createElement(QString("Unspecified"));
  }

  return requestedTaxonomy;
}

//-----------------------------------------------------------------------------
TaxonomyElementSPtr Taxonomy::createElement(const QString& name,
                                            TaxonomyElementSPtr parent)
{
  return createElement(name, parent.get());
}

//-----------------------------------------------------------------------------
void Taxonomy::deleteElement(TaxonomyElementPtr element)
{
  Q_ASSERT(element);

  if (element != m_root.get())
  {
    TaxonomyElementPtr parentElement = element->parent();
    parentElement->deleteElement(element);
  }
  else
    m_root.reset();
}

//-----------------------------------------------------------------------------
void Taxonomy::deleteElement(TaxonomyElementSPtr element)
{
  deleteElement(element.get());
}

//-----------------------------------------------------------------------------
TaxonomyElementSPtr Taxonomy::element(const QString& qualifiedName)
{
  QStringList path = qualifiedName.split("/", QString::SkipEmptyParts);
  TaxonomyElementSPtr node = m_root;

  int i = 0;
  while (node.get() != NULL && i < path.length())
  {
    node = node->element(path[i]);
    ++i;
  }

  if (node == m_root)
    node.reset();

  return node;
}

//-----------------------------------------------------------------------------
TaxonomyElementSPtr Taxonomy::parent(const TaxonomyElementSPtr element) const
{
  QStringList path = element->qualifiedName().split("/", QString::SkipEmptyParts);

  TaxonomyElementSPtr parent = m_root;
  for(int i = 0; i < path.length() - 1; i++)
  {
    parent = parent->element(path[i]);
    Q_ASSERT(parent.get() != NULL);
  }

  return parent;
}

//-----------------------------------------------------------------------------
void Taxonomy::print(int indent)
{
  m_root->print(indent);
}
