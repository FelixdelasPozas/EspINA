/*

    Copyright (C) 2014  Jaime Fernandez <jfernandez@cesvima.upm.es>
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

#ifndef ESPINA_CATEGORY_ADAPTER_H
#define ESPINA_CATEGORY_ADAPTER_H

// ESPINA
#include "ItemAdapter.h"

// Qt
#include <QVariant>
#include <QColor>


namespace ESPINA
{
  class CategoryAdapter;
  using CategoryAdapterPtr   = CategoryAdapter *;
  using CategoryAdapterList  = QList<CategoryAdapterPtr>;
  using CategoryAdapterSPtr  = std::shared_ptr<CategoryAdapter>;
  using CategoryAdapterSList = QList<CategoryAdapterSPtr>;

  /** \class CategoryAdapter.
   * \brief Category for taxons.
   *
   *  Represent a group of individuals with the same characteristics
   */
  class EspinaGUI_EXPORT CategoryAdapter
  : public ItemAdapter
  {
  public:
    struct AlreadyDefinedCategoryException {};

  public:
    /** \brief CategoryAdapter class destructor.
     *
     */
    ~CategoryAdapter();

    /** \brief Implements ItemAdapter::data() const.
     *
     */
    virtual QVariant data(int role = Qt::DisplayRole) const;

    /** \brief Implements ItemAdapter::setData().
     *
     */
    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    /** \brief Implements ItemAdapter::type() const.
     *
     */
    virtual Type type() const
    { return Type::CATEGORY; }

    /** \brief Specify the name of the category.
     * \param[in] name, name of the category.
     *
     */
    void setName(const QString &name);

    /** \brief Return the name of the category.
     *
     */
    QString name() const;

    /** \brief Return the name of the category inside a classification.
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
     * \param[in] color.
     *
     */
    void setColor(const QColor &color);

    /** \brief Returns the color of the category.
     *
     */
    QColor color() const;

    /** \brief Adds a property and a value to the category.
     * \param[in] prop, property key.
     * \param[in] value, property value.
     *
     */
    void addProperty   (const QString &prop, const QVariant &value);

    /** \brief Removes a property from the category.
     * \param[in] prop, property key.
     *
     */
    void deleteProperty(const QString &prop);

    /** \brief Returns the value of the specified property.
     * \param[in] prop, property key.
     *
     */
    QVariant property(const QString &prop) const;

    /** \brief Returns a list of the properties the category has.
     *
     */
    QStringList properties() const;

    /** \brief Create a new sub category with the given name.
     * \param[in] name, sub-category name.
     *
     */
    CategoryAdapterSPtr createSubCategory(const QString &name);

    /** \brief Make sub-category a sub category of this one.
     * \param[in] sub-category, category adapter smart pointer.
     *
     *  If the sub-category belonged to another category, it won't belong
     *  anymore.
     */
    void addSubCategory(CategoryAdapterSPtr subCategory);

    /** \brief Remove sub-category from this category.
     * \param[in] subCategory, raw pointer of the sub-category to remove.
     *
     *  If the sub-category doesn't belong to this category
     *  nothing will happen.
     */
    void removeSubCategory(CategoryAdapterPtr  subCategory);

    /** \brief Remove sub-category from this category.
     * \param[in] subCategory, smart pointer of the sub-category to remove.
     *
     *  If the sub-category doesn't belong to this category
     *  nothing will happen.
     */
    void removeSubCategory(CategoryAdapterSPtr subCategory)
    { removeSubCategory(subCategory.get()); }

    /** \brief Return the sub-category with the given name.
     * \param[in] name, name of the sub-category to return the smart pointer.
     *
     *  If no sub-category has the requested name, nullptr will be returned
     */
    CategoryAdapterSPtr subCategory(const QString &name) const;

    /** \brief Return a list with all the sub-categories of this category.
     *
     */
    CategoryAdapterSList subCategories()
    {return m_subCategories;}

    /** \brief Return a list with all the sub-categories of this category.
     *
     */
    const CategoryAdapterSList subCategories() const
    {return m_subCategories;}

    /** \brief Return the category from which this is a sub-category, if any.
     *
     * WARNING: Shadows QObject::parent().
     *
     */
    CategoryAdapterPtr parent()
    {return m_parent;}

  private:
    /** \brief CategoryAdapter class constructor.
     * \param[in] category, smart pointer of the category to adapt.
     *
     */
    explicit CategoryAdapter(CategorySPtr category);

    /** \brief CategoryAdapter class constructor.
     * \param[in] parent of the adapted category to create.
     * \param[in] name, name of the category to create.
     *
     */
    explicit CategoryAdapter(CategoryAdapterPtr parent, const QString& name);

  private:
    CategorySPtr         m_category;
    CategoryAdapterPtr   m_parent; // Parent node can't be a shared pointer to avoid circular dependencies
    CategoryAdapterSList m_subCategories;

    friend class ClassificationAdapter;
    friend class SegmentationAdapter;
    template<typename T> friend class Tree;
    friend QString print(CategoryAdapterSPtr category, int level);
  };

  /** \brief Returns the raw pointer of the category specified by the model index.
   * \param[in] index, model index.
   *
   */
  CategoryAdapterPtr EspinaGUI_EXPORT categoryPtr(const QModelIndex& index);

  /** \brief Returns the category adapter raw pointer of the item.
   * \param[in] item, item adapter raw pointer.
   *
   */
  CategoryAdapterPtr EspinaGUI_EXPORT categoryPtr(ItemAdapterPtr item);

  /** \brief Prints the category and its sub-categories with the specified indentation.
   * \param[in] category, category adapter smart pointer.
   * \param[in] indent, indentation value.
   *
   */
  QString EspinaGUI_EXPORT print(CategoryAdapterSPtr category, int indent = 0);
}// namespace ESPINA

#endif // ESPINA_CATEGORY_ADAPTER_H
