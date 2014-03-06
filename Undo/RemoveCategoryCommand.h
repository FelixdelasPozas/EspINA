/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2013  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "EspinaUndo_Export.h"
#include <QUndoCommand>

#include <GUI/Model/ModelAdapter.h>

namespace EspINA
{
  // Remove Taxonomical Element from model
  class EspinaUndo_EXPORT RemoveCategoryCommand
  : public QUndoCommand
  {
  public:
    explicit RemoveCategoryCommand(CategoryAdapterPtr category,
                                   ModelAdapterSPtr   model,
                                   QUndoCommand*      parent=nullptr);

    virtual ~RemoveCategoryCommand();

    virtual void redo();

    virtual void undo();

  private:
    ModelAdapterSPtr m_model;

    CategoryAdapterSPtr m_category;
    CategoryAdapterSPtr m_parent;
  };

} // namespace EspINA

#endif // TAXONOMIESCOMMAND_H
