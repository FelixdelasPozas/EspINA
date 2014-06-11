/*
    <one line to give the program's name and a brief idea of what it does.>
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


#ifndef ESPINA_ADD_RELATION_COMMAND_H
#define ESPINA_ADD_RELATION_COMMAND_H

#include "EspinaUndo_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/ItemAdapter.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class EspinaUndo_EXPORT AddRelationCommand
  : public QUndoCommand
  {
  public:
    explicit AddRelationCommand(ItemAdapterSPtr  ancestor,
                                ItemAdapterSPtr  successor,
                                const QString   &relationName,
                                ModelAdapterSPtr model,
                                QUndoCommand    *parent = 0);

    virtual void redo();
    virtual void undo();

  private:
    ItemAdapterSPtr  m_ancester;
    ItemAdapterSPtr  m_succesor;
    QString          m_relation;
    ModelAdapterSPtr m_model;
  };

}// namespace EspINA

#endif // ESPINA_ADD_RELATION_COMMAND_H
