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


#ifndef ESPINA_CATEGORY_SELECTOR_H
#define ESPINA_CATEGORY_SELECTOR_H

#include <QWidgetAction>

#include <QLabel>
#include <QSpinBox>
#include <GUI/Model/ModelAdapter.h>

namespace EspINA
{

  class CategorySelector
  : public QWidgetAction
  {
    Q_OBJECT
  public:
    explicit CategorySelector(ModelAdapterSPtr model,
                              QObject         *parent=nullptr);

    virtual QWidget* createWidget(QWidget* parent);

    void selectCategory(CategoryAdapterSPtr category);

    CategoryAdapterSPtr selectedCategory();

  signals:
    void categoryChanged(CategoryAdapterSPtr);
    void widgetCreated();

  private slots:
    void categorySelected(const QModelIndex& index);
    void onWidgetDestroyed(QObject* object);
    void invalidateState();
    void resetRootItem();

  private:
    ModelAdapterSPtr m_model;
    QList<QObject *> m_pool;

    CategoryAdapterSPtr m_selectedCategory;
  };

} // namespace EspINA

#endif // ESPINA_CATEGORY_SELECTOR_H
