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

#ifndef ESPINA_CATEGORY_H
#define ESPINA_CATEGORY_H

#include "Core/EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Utils/Tree.h"

// Qt dependencies
#include <QColor>
#include <QMap>
#include <QString>
#include <QList>
#include <QTextStream>
#include <QVariant>

namespace EspINA
{
  const QString DEFAULT_CATEGORY_COLOR = "#00FF00"; //Red

  typedef QList<CategorySPtr> CategorySList;

  /** \brief Category for taxons
   * 
   *  Represent a group of individuals with the same characteristics
   */
  //TODO 2013-10-21: Mark edit operations as private, except for Classification
  class EspinaCore_EXPORT Category
  {
//   public:
//     static const QString X_DIM;
//     static const QString Y_DIM;
//     static const QString Z_DIM;

  public:
    struct AlreadyDefinedCategoryException {};

  public:
    ~Category();

    /** \brief Specify the name of the category 
     * 
     */
    void setName(const QString &name);
    /** \brief Return the name of the category 
     * 
     */
    QString name() const;

    /** \brief Return the name of the category inside a classification
     * 
     *  One classification may use different categories with the same name
     *  while they are not defined at the same level of the classification.
     *  These categories will have the same name but different classification
     *  names.\n
     * 
     *  A classification name is the concatenation of the names of all the 
     *  categories from the root of the classification to the category itself
     */
    QString classificationName() const;

    void setColor(const QColor &color);

    QColor color() const {return m_color;}

    /** \brief Add a 
     * 
     */
    void addProperty   (const QString &prop, const QVariant &value);
    void deleteProperty(const QString &prop);

    QVariant    property(const QString &prop) const;
    QStringList properties() const
    {return m_properties.keys();}

    /** \brief Create a new sub category with the given name
     * 
     */
    CategorySPtr createSubCategory(const QString &name);

    /** \brief Make sub-category a sub category of this category
     * 
     *  If the sub-category belonged to another category, it won't belong
     *  anymore
     */
    void addSubCategory(CategorySPtr subCategory);

    /** \brief Remove sub-category from this category
     * 
     *  If the sub-category doesn't belong to this category
     *  nothing will happen
     */
    void removeSubCategory(CategoryPtr  subCategory);
    void removeSubCategory(CategorySPtr subCategory)
    { removeSubCategory(subCategory.get()); }

    /** \brief Return the sub-category with the given name.
     * 
     *  If no sub-category has the requested name, nullptr will be returned
     */
    CategorySPtr subCategory(const QString &name) const;

    /** \brief Return a list with all the sub-categories of this category
     * 
     */
    CategorySList subCategories()
    {return m_subCategories;}

    /** \brief Return a list with all the sub-categories of this category
     * 
     */
    const CategorySList subCategories() const
    {return m_subCategories;}

    /** \brief Return the category from which this is a sub-category, if any.
     * 
     */
    CategoryPtr parent() {return m_parent;}
  private:
    explicit Category(CategoryPtr parent,
                      const QString &name,
                      const QString &RGBColor = DEFAULT_CATEGORY_COLOR );

  private:
    CategoryPtr    m_parent; // Parent node can't be a shared pointer to avoid circular dependencies
    CategorySList  m_subCategories;

    QString m_name;
    QColor  m_color;
    QMap<QString, QVariant> m_properties;

    template<typename T> friend class Tree;
  };

  QString print(CategorySPtr category, int level=0);

  using Classification     = Tree<Category>;
  using ClassificationSPtr = std::shared_ptr<Classification>;
}// namespace EspINA

#endif // ESPINA_CATEGORY_H
