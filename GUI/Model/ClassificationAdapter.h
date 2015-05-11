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

#ifndef ESPINA_CLASSIFICATION_ADAPTER_H
#define ESPINA_CLASSIFICATION_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Model/ItemAdapter.h"
#include "GUI/Model/CategoryAdapter.h"
#include <Core/Analysis/Category.h>

// Qt
#include <QColor>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVariant>

namespace ESPINA
{
  class CategoryAdapter;
  using CategoryAdapterPtr   = CategoryAdapter *;
  using CategoryAdapterSPtr  = std::shared_ptr<CategoryAdapter>;
  using CategoryAdapterSList = QList<CategoryAdapterSPtr>;

  /// Tree-like structure representing taxonomical relationships
  class EspinaGUI_EXPORT ClassificationAdapter
  : public ItemAdapter
  {
    static const QString ROOT;

  public:
    /** \brief ClassificationAdapter class constructor.
     * \param[in] name name of the classification.
     *
     */
    explicit ClassificationAdapter(const QString& name = "Undefined");

    /** \brief ClassificationAdapter class constructor.
     * \param[in] classification smart pointer of the classification to adapt.
     *
     */
    explicit ClassificationAdapter(ClassificationSPtr classification);

    /** \brief ClassificationAdapter class destructor.
     *
     */
    ~ClassificationAdapter();

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual ItemAdapter::Type type() const
    { return Type::CLASSIFICATION; }

    /** \brief Sets the name of the classification.
     * \param[in] name name of the classification.
     *
     */
    void setName(const QString& name);

    /** \brief Returns the name of the classification.
     *
     */
    QString name() const;

    /** \brief Creates a category inside the classification.
     * \param[in] relativeName name of the category.
     * \param[in] parent smart pointer of the parent category of the one created.
     *
     */
    CategoryAdapterSPtr createCategory(const QString &relativeName,
                                       CategoryAdapterSPtr parent = CategoryAdapterSPtr());

    /** \brief Removes a category from the classification.
     * \param[in] element smart pointer of the category adapter to remove.
     *
     */
    void removeCategory(CategoryAdapterSPtr element);

    /** \brief Returns the smart pointer of the root node of the classification.
     *
     */
    CategoryAdapterSPtr root();

    /** \brief Returns the smart pointer of the category with the specified name.
     * \param[in] categoryName name of the category to return.
     *
     */
    CategoryAdapterSPtr category(const QString &categoryName);

    /** \brief Returns the list of smart pointer of all the categories in the classification.
     *
     */
    CategoryAdapterSList categories();

    /** \brief Returns the smart pointer of the parent of the specified category.
     * \param[in] category category adapter smart pointer.
     *
     */
    CategoryAdapterSPtr parent(const CategoryAdapterSPtr category) const;

  private:
    /** \brief Adds the category and all its sub-categories in the classification.
     * \param[in] category category adapter smart pointer.
     *
     */
    void adaptCategory(CategoryAdapterSPtr category);

  private:
    ClassificationSPtr    m_classification;
    Tree<CategoryAdapter> m_classificationAdapter;

    friend class ModelAdapter;
  };

  using ClassificationAdapterSPtr = std::shared_ptr<ClassificationAdapter>;

  /** \brief Prints the classification with the specified indentation.
   * \param[in] classification, smart pointer of the classification to print.
   * \param[in] indent, indentation value.
   *
   */
  QString EspinaGUI_EXPORT print(ClassificationAdapterSPtr classification, int indent = 0);
}// namespace ESPINA

#endif // ESPINA_CLASSIFICATION_ADAPTER_H
