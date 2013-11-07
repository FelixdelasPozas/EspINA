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

#ifndef ESPINA_CLASSIFICATION_ADAPTER_H
#define ESPINA_CLASSIFICATION_ADAPTER_H

#include "EspinaGUI_Export.h"

#include "GUI/Model/ItemAdapter.h"
#include "GUI/Model/CategoryAdapter.h"
#include <Core/Analysis/Category.h>


// Qt dependencies
#include <QColor>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVariant>

namespace EspINA
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
    ~ClassificationAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual ItemAdapter::Type type() const
    { return Type::CLASSIFICATION; }

    void setName(const QString& name);

    QString name() const;

    CategoryAdapterSPtr createCategory(const QString &relativeName,
                                       CategoryAdapterSPtr parent = CategoryAdapterSPtr());
    void removeCategory(CategorySPtr element);

    CategoryAdapterSPtr root();

    CategoryAdapterSPtr category(const QString &classificationName);

    CategoryAdapterSList categories();

    CategoryAdapterSPtr parent(const CategoryAdapterSPtr categor) const;

  protected:
    virtual PersistentSPtr item() const;

  private:
    explicit ClassificationAdapter(ClassificationSPtr classification);

  private:
    ClassificationSPtr    m_classification;
    Tree<CategoryAdapter> m_adaptedClassification;
  };

  using ClassificationAdapterSPtr = std::shared_ptr<ClassificationAdapter>;

  QString print(ClassificationSPtr classification, int indent = 0);
}// namespace EspINA

#endif // ESPINA_CLASSIFICATION_ADAPTER_H