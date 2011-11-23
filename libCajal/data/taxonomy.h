#ifndef _TAXONOMY_
#define _TAXONOMY_

#include "modelItem.h"

#include <vector>

// Qt dependencies
#include <QString>
#include <QXmlStreamWriter> //TODO move to cpp
#include <QTextStream>
#include <QColor>

static const QString RED = "#00FF00";

class TaxonomyNode : public IModelItem
{
public:
  TaxonomyNode(const QString &name, const QString &RGBColor = RED );
  ~TaxonomyNode();
  
  TaxonomyNode *addElement(const QString& qualifiedName);
  /// Return taxonomy node for qualified taxonomy elements
  TaxonomyNode* element(const QString &name);
  TaxonomyNode *parentNode();
  QVector<TaxonomyNode*> subElements() const {return m_elements;}
  
  void setName(QString name);
  const QString name() const;
  /// Return node's qualified name
  const QString qualifiedName() const;
  
  void setColor(const QColor &color) {m_color = color;} 
  QColor color() const {return m_color;}
  
  void addProperty(const QString &prop, const QVariant &value);
  void removeProperty(const QString &prop);
  QVariant property(const QString &prop) const;
  QStringList properties() const {return m_properties.keys();}
  
  
  void print(int level=0);   
  // It introduces the subElement string as a subnode of supElement string. If subElement
  // exists in all the tree that this object has it returns an Error.
  // Note that to check the existence of the subElement name, it is necesarry to insert all 
  // the elements through the TaxonoyNode object at the top of the tree.
  TaxonomyNode* addElement( QString subElement, QString supElement);//, QString RGBColor = "");
  void removeChild(QString name);
  
  //! Implements IModelItem
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole + 1);
  
private:
//  TaxonomyNode *insertElement( QString subElement, QString RGBColor ); // Without checking
 TaxonomyNode *insertNode(const QString &name);
 
private:
 TaxonomyNode *m_parent;
 QVector<TaxonomyNode *> m_elements;
 QMap<QString, QVariant> m_properties;
 QString m_name;
 QColor m_color;
};



class Taxonomy
{
public:
  Taxonomy(const QString &name);
  ~Taxonomy();
  
  QString name() {return m_root->name();}
  
  TaxonomyNode *addElement(const QString& name, const QString& parent = QString());
  void removeElement(const QString &qualifiedName);
  TaxonomyNode *element(const QString&qualifiedName);
  QVector<TaxonomyNode *> elements() {return m_root->subElements();}
  
  void print(int indent = 0);  
  
private:
  TaxonomyNode *m_root;
};


class IOTaxonomy
{
private:
  static void writeTaxonomy(Taxonomy* tax, QXmlStreamWriter& stream);
  static void writeTaxonomyNode(TaxonomyNode* node, QXmlStreamWriter& stream);
  static Taxonomy* readXML( QXmlStreamReader& xmlStream );
public:
  IOTaxonomy();
  ~IOTaxonomy();

  static Taxonomy* openXMLTaxonomy( QString fileName);
  static Taxonomy* loadXMLTaxonomy( QString& content);

  //static void writeXMLTaxonomy(TaxonomyNode& tax, QString fileName);
  static void writeXMLTaxonomy(Taxonomy* tax, QString& destination);
};

#endif // _TAXONOMY_
