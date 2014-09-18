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

#ifndef ESPINA_ADD_CLASSIFICATION_COMMAND_H
#define ESPINA_ADD_CLASSIFICATION_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <Core/Analysis/Category.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{

  class EspinaUndo_EXPORT AddClassificationCommand
  : public QUndoCommand
  {
  public:
  	/* \brief AddClassificationCommand class constructor.
  	 * \param[in] classification, smart pointer of the classification adapter to add.
  	 * \param[in] model, raw pointer of the model adapter.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit AddClassificationCommand(ClassificationAdapterSPtr  classification,
                                      ModelAdapter              *model,
                                      QUndoCommand              *parent = nullptr);

    /* \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /* \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    /* \brief Helper method to swap new-old classifications.
     *
     */
    void swapClassification();

  private:
    ModelAdapter *m_model;

    ClassificationAdapterSPtr m_prevClassification;
  };

} // ESPINA

#endif // ESPINA_ADD_CLASSIFICATION_COMMAND_H
