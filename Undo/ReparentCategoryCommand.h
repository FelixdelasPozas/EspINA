/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_REPARENT_CATEGORY_COMMAND_H
#define ESPINA_REPARENT_CATEGORY_COMMAND_H

#include <QUndoCommand>
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA {

  class ReparentCategoryCommand
  : public QUndoCommand
  {
  public:
    explicit ReparentCategoryCommand(CategoryAdapterPtr category,
                                     CategoryAdapterPtr parentCategory,
                                     ModelAdapterSPtr   model,
                                     QUndoCommand*      parent = nullptr);

    explicit ReparentCategoryCommand(CategoryAdapterList categories,
                                     CategoryAdapterPtr  parentCategory,
                                     ModelAdapterSPtr    model,
                                     QUndoCommand*       parent = nullptr);

    virtual ~ReparentCategoryCommand();

    virtual void redo();

    virtual void undo();

  private:
    void storePreviousParent(CategoryAdapterPtr category);

  private:
    ModelAdapterSPtr m_model;

    CategoryAdapterSPtr m_parent;
    QMap<CategoryAdapterSPtr, CategoryAdapterSPtr> m_previousParents;
  };
}

#endif // ESPINA_REPARENTCATEGORYCOMMAND_H
