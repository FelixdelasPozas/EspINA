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

#ifndef ESPINA_CATEGORY_ADAPTER_H
#define ESPINA_CATEGORY_ADAPTER_H

#include "ItemAdapter.h"

#include <QVariant>
#include <QColor>


namespace EspINA
{
  class CategoryAdapter;
  using CategoryAdapterPtr   = CategoryAdapter *;
  using CategoryAdapterSPtr  = std::shared_ptr<CategoryAdapter>;
  using CategoryAdapterSList = QList<CategoryAdapterSPtr>;

  /** \brief Category for taxons
   *
   *  Represent a group of individuals with the same characteristics
   */
  class EspinaCore_EXPORT CategoryAdapter
  : public ItemAdapter
  {
  public:
    struct AlreadyDefinedCategoryException {};

  public:
    ~CategoryAdapter();

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

    QColor color() const;

    /** \brief Add a 
     * 
     */
    void addProperty   (const QString &prop, const QVariant &value);

    void deleteProperty(const QString &prop);

    QVariant    property(const QString &prop) const;

    QStringList properties() const;

    /** \brief Create a new sub category with the given name
     * 
     */
    CategoryAdapterSPtr createSubCategory(const QString &name);

    /** \brief Make sub-category a sub category of this category
     * 
     *  If the sub-category belonged to another category, it won't belong
     *  anymore
     */
    void addSubCategory(CategoryAdapterSPtr subCategory);

    /** \brief Remove sub-category from this category
     * 
     *  If the sub-category doesn't belong to this category
     *  nothing will happen
     */
    void removeSubCategory(CategoryAdapterPtr  subCategory);

    void removeSubCategory(CategoryAdapterSPtr subCategory)
    { removeSubCategory(subCategory.get()); }

    /** \brief Return the sub-category with the given name.
     * 
     *  If no sub-category has the requested name, nullptr will be returned
     */
    CategoryAdapterSPtr subCategory(const QString &name) const;

    /** \brief Return a list with all the sub-categories of this category
     * 
     */
    CategoryAdapterSList subCategories()
    {return m_subCategories;}

    /** \brief Return a list with all the sub-categories of this category
     * 
     */
    const CategoryAdapterSList subCategories() const
    {return m_subCategories;}

    /** \brief Return the category from which this is a sub-category, if any.
     * 
     */
    CategoryAdapterPtr parent() {return m_parent;}

  protected:
    virtual PersistentSPtr item() const;

  private:
    explicit CategoryAdapter(CategorySPtr category);

  private:
    CategorySPtr         m_category;
    CategoryAdapterPtr   m_parent; // Parent node can't be a shared pointer to avoid circular dependencies
    CategoryAdapterSList m_subCategories;

    friend class Classification;
  };

  QString print(CategoryAdapterSPtr category, int level=0);
}// namespace EspINA

#endif // ESPINA_CATEGORY_ADAPTER_H
