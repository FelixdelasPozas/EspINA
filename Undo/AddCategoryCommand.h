/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_ADD_CATEGORY_COMMAND_H
#define ESPINA_ADD_CATEGORY_COMMAND_H

#include "EspinaUndo_Export.h"
#include <QUndoStack>

#include <GUI/Model/ModelAdapter.h>

namespace EspINA {

  class EspinaUndo_EXPORT AddCategoryCommand
  : public QUndoCommand
  {
  public:
    explicit AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                CategoryAdapterSPtr category,
                                ModelAdapterSPtr    model,
                                QUndoCommand*       parent = nullptr);

    explicit AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                const QString&      name,
                                ModelAdapterSPtr    model,
                                QColor              color,
                                QUndoCommand*       parent = nullptr);
    virtual ~AddCategoryCommand();

    virtual void redo();

    virtual void undo();

  private:
    ModelAdapterSPtr    m_model;
    QString             m_name;
    QColor              m_color;
    CategoryAdapterSPtr m_category;
    CategoryAdapterSPtr m_parentCategory;
  };

} // EspINA

#endif // ESPINA_ADD_CATEGORY_COMMAND_H
