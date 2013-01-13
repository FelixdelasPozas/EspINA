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

#include "Core/Model/ModelItem.h"
#include "Core/Model/HierarchyItem.h"

// Qt dependencies
#include <QColor>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVariant>

namespace EspINA
{
  const QString DEFAULT_TAXONOMY_COLOR = "#00FF00"; //Red

  typedef QSharedPointer<TaxonomyElement> TaxonomyElementSPtr;
  typedef QList<TaxonomyElementSPtr>      TaxonomyElementSList;

  class TaxonomyElement
  : public ModelItem
  , public HierarchyItem
  {
    Q_OBJECT
  public:
    static const QString X_DIM;
    static const QString Y_DIM;
    static const QString Z_DIM;

  public:
    ~TaxonomyElement();

    /// Implements ModelItem
    virtual void initialize(const Arguments &args = Arguments()){};
    virtual void initializeExtensions(const Arguments &args = Arguments()){};
    virtual QVariant data(int role = Qt::DisplayRole) const;
    virtual QString serialize() const {return ModelItem::serialize();}
    virtual ModelItemType type() const {return TAXONOMY;}
    virtual bool setData(const QVariant& value, int role = Qt::UserRole + 1);

    void setName(const QString &name);
    QString name() const;

    /// Return the concatenation of the names of all elments from the
    /// root to this element
    QString qualifiedName() const;

    void setColor(const QColor &color);

    QColor color() const {return m_color;}

    void addProperty   (const QString &prop, const QVariant &value);
    void removeProperty(const QString &prop);

    QVariant    property(const QString &prop) const;
    QStringList properties() const
    {return m_properties.keys();}

    /// Create a new sub-element
    TaxonomyElementSPtr createElement(const QString &name);
    /// Add sub-element
    void addElement(TaxonomyElementSPtr element);
    /// Delete element only if it is its sub-element
    void deleteElement(TaxonomyElementPtr element);

    /// Return sub-element with given name, otherwise return NULL
    TaxonomyElementSPtr element(const QString &name);

    TaxonomyElementSList subElements()
    {return m_elements;}
    const TaxonomyElementSList subElements() const
    {return m_elements;}
    TaxonomyElementPtr parent() {return m_parent;}

    void print(int level=0);

  signals:
    void colorChanged(TaxonomyElementPtr);

  private:
    explicit TaxonomyElement(TaxonomyElementPtr parent,
                             const QString &name,
                             const QString &RGBColor = DEFAULT_TAXONOMY_COLOR );

  private:
    TaxonomyElementPtr         m_parent; // Parent node can't be a shared pointer to avoid circular dependencies
    TaxonomyElementSList  m_elements;

    QString m_name;
    QColor  m_color;
    QMap<QString, QVariant> m_properties;

    friend class Taxonomy;
  };

  TaxonomyElementPtr  taxonomyElementPtr(ModelItemPtr   item);
  TaxonomyElementSPtr taxonomyElementPtr(ModelItemSPtr &item);


  typedef QSharedPointer<Taxonomy> TaxonomySPtr;
  /// Tree-like structure representing taxonomical relationships
  class Taxonomy
  {
    static const QString ROOT;

  public:
    explicit Taxonomy();
    ~Taxonomy();

    TaxonomyElementSPtr createElement(const QString &name,
                                           TaxonomyElementPtr parent);
    TaxonomyElementSPtr createElement(const QString &name,
                                           TaxonomyElementSPtr parent = TaxonomyElementSPtr());

    void deleteElement(TaxonomyElementPtr element);
    void deleteElement(TaxonomyElementSPtr element);


    TaxonomyElementSPtr  root(){return m_root;}
    TaxonomyElementSPtr  element(const QString &qualifiedName);
    TaxonomyElementSList elements() {return m_root->subElements();}
    TaxonomyElementSPtr  parent(const TaxonomyElementSPtr element) const;

    void print(int indent = 0);

  private:
    TaxonomyElementSPtr m_root;
  };

}// namespace EspINA

#endif // TAXONOMY_H
