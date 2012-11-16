/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jaime Fernandez <jfernandez@cesvima.upm.es>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#ifndef TAXONOMY_H
#define TAXONOMY_H

#include "ModelItem.h"

// Qt dependencies
#include <QColor>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVariant>
#include <QVector>

// Forward-declaration
class QXmlStreamReader;
class QXmlStreamWriter;

static const QString RED = "#00FF00";

class TaxonomyElement
: public ModelItem
{
public:
  static const QString X_DIM;
  static const QString Y_DIM;
  static const QString Z_DIM;

public:
  explicit TaxonomyElement(const QString name, const QString RGBColor = RED );
  ~TaxonomyElement();

  /// Add a new node at the location specified by @qualifiedName
  TaxonomyElement *addElement(const QString qualifiedName);
  /// Return taxonomy node for qualified taxonomy elements
  TaxonomyElement* element(const QString qualifiedName);

  TaxonomyElement *parentNode() const;
  QVector<TaxonomyElement*> subElements() const {return m_elements;}

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
  TaxonomyElement* addElement( QString subElement, QString supElement);//, QString RGBColor = "");
  void removeChild(QString name);

  /// Implements ModelItem
  virtual QString id() const {return qualifiedName();}
  virtual void initialize(Arguments args = Arguments()){};
  virtual void initializeExtensions(Arguments args = Arguments()){};
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual QString serialize() const {return ModelItem::serialize();}
  virtual ItemType type() const {return TAXONOMY;}
  virtual bool setData(const QVariant& value, int role = Qt::UserRole + 1);

private:
//  TaxonomyNode *insertElement( QString subElement, QString RGBColor ); // Without checking
 TaxonomyElement *insertNode(const QString &name);

private:
 TaxonomyElement *m_parent;
 QVector<TaxonomyElement *> m_elements;
 QMap<QString, QVariant> m_properties;
 QString m_name;
 QColor m_color;
};


class Taxonomy
{
public:
  Taxonomy();
  ~Taxonomy();

//   QString name() {return m_root->name();}

  TaxonomyElement *addElement(const QString name, const QString parent = QString());
  void removeElement(const QString qualifiedName);
  TaxonomyElement *element(const QString qualifiedName);
  TaxonomyElement *root(){return m_root;}
  QVector<TaxonomyElement *> elements() {return m_root->subElements();}

  void print(int indent = 0);

private:
  TaxonomyElement *m_root;
};

class IOTaxonomy
{
public:
  static Taxonomy *openXMLTaxonomy(QString fileName);
  static Taxonomy *loadXMLTaxonomy(QString content);
  static void writeXMLTaxonomy(Taxonomy *tax, QString& destination);

private:
  IOTaxonomy();
  ~IOTaxonomy();

  static void writeTaxonomy(Taxonomy *tax, QXmlStreamWriter& stream);
  static void writeTaxonomyElement(TaxonomyElement *node, QXmlStreamWriter& stream);
  static Taxonomy *readXML(QXmlStreamReader &xmlStream);

  //static void writeXMLTaxonomy(TaxonomyNode& tax, QString fileName);
};

#endif // TAXONOMY_H