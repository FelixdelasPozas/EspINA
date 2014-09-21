/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_REMOVE_CATEGORY_COMMAND_H
#define ESPINA_REMOVE_CATEGORY_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
	/** \class RemoveCategoryCommand.
	 * \brief Remove Taxonomical Element from model
	 *
	 */
  class EspinaUndo_EXPORT RemoveCategoryCommand
  : public QUndoCommand
  {
  public:
  	/** brief RemoveCategoryCommand class constructor.
  	 * \param[in] category, raw pointer of the category adapter to remove.
  	 * \param[in] model, smart pointer of the model adapter containing the category.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 */
    explicit RemoveCategoryCommand(CategoryAdapterPtr category,
                                   ModelAdapterSPtr   model,
                                   QUndoCommand*      parent = nullptr);

    /** brief RemoveCategoryCommand class destructor.
     *
     */
    virtual ~RemoveCategoryCommand();

    /** brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    ModelAdapterSPtr m_model;

    CategoryAdapterSPtr m_category;
    CategoryAdapterSPtr m_parent;
  };

} // namespace ESPINA

#endif // TAXONOMIESCOMMAND_H
