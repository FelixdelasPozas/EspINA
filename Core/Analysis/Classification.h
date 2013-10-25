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

#ifndef ESPINA_CLASSIFICATION_H
#define ESPINA_CLASSIFICATION_H

#include "EspinaCore_Export.h"

#include "Core/Analysis/Category.h"


// Qt dependencies
#include <QColor>
#include <QMap>
#include <QString>
#include <QTextStream>
#include <QVariant>

namespace EspINA
{
  //const QString DEFAULT_CATEGORY_COLOR = "#00FF00"; //Red

  /// Tree-like structure representing taxonomical relationships
  class EspinaCore_EXPORT Classification
  {
    static const QString ROOT;

  public:
    explicit Classification(const QString& name=QString());
    ~Classification();

    void setName(const QString& name)
    { m_name = name; }

    QString name() const
    { return m_name; }

    CategorySPtr createCategory(const QString &relativeName,
                                CategorySPtr parent = CategorySPtr());

    void removeCategory(CategorySPtr element);

    CategorySPtr  root(){return m_root;}
    CategorySPtr  category(const QString &classificationName);
    CategorySList categories() {return m_root->subCategories();}
    CategorySPtr  parent(const CategorySPtr categor) const;

  private:
    QString      m_name;
    CategorySPtr m_root;
  };

  QString print(ClassificationSPtr classification, int indent = 0);
}// namespace EspINA

#endif // ESPINA_CLASSIFICATION_H
