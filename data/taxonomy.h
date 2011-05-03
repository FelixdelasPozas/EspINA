#ifndef _TAXONOMY_
#define _TAXONOMY_

#include "modelItem.h"

#include <vector>

// Qt dependencies
#include <QString>
#include <QXmlStreamWriter> //TODO move to cpp
#include <QTextStream>
#include <QColor>

class TaxonomyNode : public IModelItem
{
 
public:
  TaxonomyNode( QString name, QString RGBColor = "#00FF00" );
  ~TaxonomyNode();
  
  void print(int level=0);   
  //void addElement( QString subElement ); // With checking. TO DELTE. No tiene sentido estando el otro addElement
  
  // It introduces the subElement string as a subnode of supElement string. If subElement
  // exists in all the tree that this object has it returns an Error.
  // Note that to check the existence of the subElement name, it is necesarry to insert all 
  // the elements through the TaxonoyNode object at the top of the tree.
  TaxonomyNode* addElement( QString subElement, QString supElement, QString RGBColor = "");
 
  // Methods to explore the taxonomy
  TaxonomyNode* getParent( QString name );
  QVector<TaxonomyNode*> getSubElements() const;
  TaxonomyNode* getComponent( QString name ); 
  
  // Taxonomy information methods
  QString getName() const {return m_name;}
  QString getDescription() const 
    {return m_description;}
  void setDescription(const QString &desc) {m_description = desc;}
  QColor getColor() const {return m_color;}
  void setColor(const QColor &color) {m_color = color;} 
  
  //! Implements IModelItem
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  
private:
 TaxonomyNode *insertElement( QString subElement, QString RGBColor ); // Without checking
 
private:
 QVector<TaxonomyNode *> m_elements;
 QString m_name, m_description;
 QColor m_color;
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
