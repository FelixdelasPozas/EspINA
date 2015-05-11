/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "Core/EspinaTypes.h"
#include "Core/Utils/Tree.hxx"

// Qt
#include <QColor>
#include <QMap>
#include <QString>
#include <QList>
#include <QTextStream>
#include <QVariant>

namespace ESPINA
{
  const Hue DEFAULT_CATEGORY_COLOR = 0; //Red

  typedef QList<CategorySPtr> CategorySList;

  /** \brief Category for taxons
   *
   *  Represent a group of individuals with the same characteristics
   *
   *  TODO 2013-10-21: Mark edit operations as private, except for Classification
   */
  class EspinaCore_EXPORT Category
  {
  public:
    static const QString DIM_X()
    { return X_DIM; }
    static const QString DIM_Y()
    { return Y_DIM; }
    static const QString DIM_Z()
    { return Z_DIM; }

   private:
     static const QString X_DIM;
     static const QString Y_DIM;
     static const QString Z_DIM;

  public:
    struct AlreadyDefinedCategoryException {};

  public:
    /** \brief Category class destructor.
     *
     */
    ~Category();

    /** \brief Sets the name of the category
     * \param[in] name new name of the category.
     *
     */
    void setName(const QString &name);

    /** \brief Returns the name of the category
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

    /** \brief Sets the color of the category.
     * \param[in] color new color of the category.
     *
     */
    void setColor(const Hue color);

    /** \brief Returns the color of the category.
     *
     */
    Hue color() const {return m_color;}

    /** \brief Adds a property to the category.
     * \param[in] prop key of the property.
     * \param[in] value value of the property.
     *
     */
    void addProperty(const QString &prop, const QVariant &value);

    /** \brief Deletes a property of the category.
     * \param[in] prop key of the property.
     *
     */
    void deleteProperty(const QString &prop);

    /** \brief Returns the value of a property of the category.
     * \param[in] prop key of the property.
     *
     */
    QVariant property(const QString &prop) const;

    /** \brief Returns the keys of the properties of the category.
     *
     */
    QStringList properties() const
    {return m_properties.keys();}

    /** \brief Create a new sub-category with the given name.
     * \param[in] name name of the sub-category to create.
     *
     */
    CategorySPtr createSubCategory(const QString &name);

    /** \brief Make sub-category a sub category of this category.
     * \param[in] subCategory category to make this one it's parent.
     *
     *  If the sub-category belonged to another category, it won't belong
     *  anymore
     */
    void addSubCategory(CategorySPtr subCategory);

    /** \brief Remove sub-category from this category.
     * \param[in] subCategory category to remove.
     *
     *  If the sub-category doesn't belong to this category
     *  nothing will happen
     */
    void removeSubCategory(CategoryPtr  subCategory);

    /** \brief Remove sub-category from this category.
     * \param[in] subCategory smart pointer of the category to remove.
     *
     *  If the sub-category doesn't belong to this category
     *  nothing will happen
     */
    void removeSubCategory(CategorySPtr subCategory)
    { removeSubCategory(subCategory.get()); }

    /** \brief Return the sub-category with the given name.
     * \param[in] name name of the category to return.
     *
     *  If no sub-category has the requested name, nullptr will be returned
     */
    CategorySPtr subCategory(const QString &name) const;

    /** \brief Return a list with all the sub-categories of this category.
     *
     */
    CategorySList subCategories()
    {return m_subCategories;}

    /** \brief Return a list with all the sub-categories of this category.
     *
     */
    const CategorySList subCategories() const
    {return m_subCategories;}

    /** \brief Return the category from which this is a sub-category, if any.
     *
     */
    CategoryPtr parent()
    {return m_parent;}
  private:
    /** \brief Category class constructor.
     * \param[in] parent parent category raw pointer.
     * \param[in] name name of the category.
     * \param[in] color color of the category.
     *
     */
    explicit Category(CategoryPtr parent,
                      const QString &name,
                      const Hue color = DEFAULT_CATEGORY_COLOR);

  private:
    CategoryPtr    m_parent; // Parent node can't be a shared pointer to avoid circular dependencies
    CategorySList  m_subCategories;

    QString m_name;
    Hue  m_color;
    QMap<QString, QVariant> m_properties;

    template<typename T> friend class Tree;
  };

  /** \brief Prints the category and it's properties indented.
   * \param[in] category category whose information want printed.
   * \param[in] level indentation level.
   *
   */
  QString EspinaCore_EXPORT print(CategorySPtr category, int level=0);

  using Classification     = Tree<Category>;
  using ClassificationSPtr = std::shared_ptr<Classification>;
}// namespace ESPINA

#endif // ESPINA_CATEGORY_H
