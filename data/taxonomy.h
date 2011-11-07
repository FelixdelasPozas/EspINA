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
  
  const QString name();
  /// Return node's qualified name
  const QString qualifiedName();
  
  
  void print(int level=0);   
  //void addElement( QString subElement ); // With checking. TO DELTE. No tiene sentido estando el otro addElement
  
  // It introduces the subElement string as a subnode of supElement string. If subElement
  // exists in all the tree that this object has it returns an Error.
  // Note that to check the existence of the subElement name, it is necesarry to insert all 
  // the elements through the TaxonoyNode object at the top of the tree.
  TaxonomyNode* addElement( QString subElement, QString supElement, QString RGBColor = "");
  void removeElement(QString subElement);
 
  // Methods to explore the taxonomy
  /// DEPRECATED
  TaxonomyNode* getParent( QString name );
  /// DEPRECATED
  QVector<TaxonomyNode*> getSubElements() const;
  /// DEPRECATED
  TaxonomyNode* getComponent( QString name ); 
  
  // Taxonomy information methods
  /// DEPRECATED
  QString getName() const {return m_name;}
  void setName(QString name) {m_name = name;}
  
  QString getDescription() const 
    {return m_description;}
  void setDescription(const QString &desc) {m_description = desc;}
  QColor getColor() const {return m_color;}
  void setColor(const QColor &color) {m_color = color;} 
  
  //! Implements IModelItem
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole + 1);
  
private:
 /// DEPRECATED
 TaxonomyNode *insertElement( QString subElement, QString RGBColor ); // Without checking
 TaxonomyNode *insertNode(const QString &name);
 
private:
 TaxonomyNode *m_parent;
 QVector<TaxonomyNode *> m_elements;
 QString m_name, m_description;
 QColor m_color;
};

class Taxonomy
{
public:
  Taxonomy(const QString &name);
  ~Taxonomy();
  
  TaxonomyNode *addElement(const QString& name, const QString& parent = QString());
  TaxonomyNode *element(const QString&qualifiedName);
  
  void print(int indent = 0);  
private:
  TaxonomyNode *m_root;
};

class IOTaxonomy
{
private:
  static void writeTaxonomyNode(TaxonomyNode* node, QXmlStreamWriter& stream);
  static TaxonomyNode* readXML( QXmlStreamReader& xmlStream );
public:
  IOTaxonomy();
  ~IOTaxonomy();

  static TaxonomyNode* openXMLTaxonomy( QString fileName);
  static TaxonomyNode* loadXMLTaxonomy( QString& content);

  //static void writeXMLTaxonomy(TaxonomyNode& tax, QString fileName);
  static void writeXMLTaxonomy(TaxonomyNode* tax, QString& destination);
};

#endif // _TAXONOMY_
