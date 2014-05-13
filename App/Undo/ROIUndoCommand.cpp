/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#include "ROIUndoCommand.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ModifyROIUndoCommand::ModifyROIUndoCommand(const ViewManagerSPtr vm, const BinaryMaskSPtr<unsigned char> mask, unsigned char value)
  : m_newROI     {nullptr}
  , m_viewManager{vm}
  , m_mask       {mask}
  , m_value      {value}
  {
    if(vm->currentROI() == nullptr)
      m_newROI = ROISPtr{new ROI{mask, value}};
  }

  //-----------------------------------------------------------------------------
  ModifyROIUndoCommand::~ModifyROIUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ModifyROIUndoCommand::redo()
  {
    if(m_newROI != nullptr)
      m_viewManager->setCurrentROI(m_newROI);
    else
      m_viewManager->currentROI()->draw(m_mask, m_value);
  }

  //-----------------------------------------------------------------------------
  void ModifyROIUndoCommand::undo()
  {
    if(m_newROI != nullptr)
      m_viewManager->setCurrentROI(nullptr);
    else
      m_viewManager->currentROI()->undo();
  }
}
