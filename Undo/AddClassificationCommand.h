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

#include "EspinaUndo_Export.h"
#include <QUndoStack>

#include <Core/Analysis/Category.h>
#include <GUI/Model/ModelAdapter.h>

namespace EspINA {

  class AddClassificationCommand
  : public QUndoCommand
  {
  public:
    explicit AddClassificationCommand(ClassificationAdapterSPtr  classification,
                                      ModelAdapter              *model,
                                      QUndoCommand              *parent = nullptr);
    virtual void redo();

    virtual void undo();

  private:
    void swapClassification();

  private:
    ModelAdapter *m_model;

    ClassificationAdapterSPtr m_prevClassification;
  };

} // EspINA

#endif // ESPINA_ADD_CLASSIFICATION_COMMAND_H
