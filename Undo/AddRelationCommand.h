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

#ifndef ESPINA_ADD_RELATION_COMMAND_H
#define ESPINA_ADD_RELATION_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/ItemAdapter.h>

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  class EspinaUndo_EXPORT AddRelationCommand
  : public QUndoCommand
  {
  public:
  	/* \brief AddRelationCommand class constructor.
  	 * \param[in] ancestor, smart pointer of the item adapter origin.
  	 * \param[in] successor, smart pointer of the item adapter destination.
  	 * \param[in] relationName, relation specifier.
  	 * \param[in] model, smart pointer of the model adapter that contains both items.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit AddRelationCommand(ItemAdapterSPtr  ancestor,
                                ItemAdapterSPtr  successor,
                                const QString   &relationName,
                                ModelAdapterSPtr model,
                                QUndoCommand    *parent = 0);

    /* \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /* \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    ItemAdapterSPtr  m_ancester;
    ItemAdapterSPtr  m_succesor;
    QString          m_relation;
    ModelAdapterSPtr m_model;
  };

}// namespace ESPINA

#endif // ESPINA_ADD_RELATION_COMMAND_H
