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

#ifndef ESPINA_CLASSIFICATION_ADAPTER_H
#define ESPINA_CLASSIFICATION_ADAPTER_H

#include "GUI/EspinaGUI_Export.h"

#include "GUI/Model/ItemAdapter.h"
#include "GUI/Model/CategoryAdapter.h"
#include <Core/Analysis/Category.h>


// Qt dependencies
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
    explicit ClassificationAdapter(const QString& name=QString());

    explicit ClassificationAdapter(ClassificationSPtr classification);

    ~ClassificationAdapter();

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual ItemAdapter::Type type() const
    { return Type::CLASSIFICATION; }

    void setName(const QString& name);

    QString name() const;

    CategoryAdapterSPtr createCategory(const QString &relativeName,
                                       CategoryAdapterSPtr parent = CategoryAdapterSPtr());
    void removeCategory(CategoryAdapterSPtr element);

    CategoryAdapterSPtr root();

    CategoryAdapterSPtr category(const QString &classificationName);

    CategoryAdapterSList categories();

    CategoryAdapterSPtr parent(const CategoryAdapterSPtr category) const;

  private:
    void adaptCategory(CategoryAdapterSPtr category);

  private:
    ClassificationSPtr    m_classification;
    Tree<CategoryAdapter> m_classificationAdapter;

    friend class ModelAdapter;
  };

  using ClassificationAdapterSPtr = std::shared_ptr<ClassificationAdapter>;

  QString print(ClassificationAdapterSPtr classification, int indent = 0);
}// namespace ESPINA

#endif // ESPINA_CLASSIFICATION_ADAPTER_H