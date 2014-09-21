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

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{

  class EspinaUndo_EXPORT ReparentCategoryCommand
  : public QUndoCommand
  {
  public:
  	/** \brief ReparentCategoryCommand class constructor.
  	 * \param[in] category, raw pointer of the category adapter to reparent.
  	 * \param[in] parentCategory, raw pointer of the category adapter that is the new parent.
  	 * \param[in] model, model adapter smart pointer that contains both categories.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit ReparentCategoryCommand(CategoryAdapterPtr category,
                                     CategoryAdapterPtr parentCategory,
                                     ModelAdapterSPtr   model,
                                     QUndoCommand*      parent = nullptr);

  	/** \brief ReparentCategoryCommand class constructor.
  	 * \param[in] categories, list of raw pointers of the category adapters to reparent.
  	 * \param[in] parentCategory, raw pointer of the category adapter that is the new parent.
  	 * \param[in] model, model adapter smart pointer that contains both categories.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit ReparentCategoryCommand(CategoryAdapterList categories,
                                     CategoryAdapterPtr  parentCategory,
                                     ModelAdapterSPtr    model,
                                     QUndoCommand*       parent = nullptr);

    /** \brief ReparentCategoryCommand class destructor.
     *
     */
    virtual ~ReparentCategoryCommand();

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    /** \brief Helper method to store the parent of the category for swapping later.
     * \param[in] category, smart pointer of the category adapter whose parent will be stored.
     *
     */
    void storePreviousParent(CategoryAdapterPtr category);

  private:
    ModelAdapterSPtr m_model;

    CategoryAdapterSPtr m_parent;
    QMap<CategoryAdapterSPtr, CategoryAdapterSPtr> m_previousParents;
  };
}

#endif // ESPINA_REPARENTCATEGORYCOMMAND_H
