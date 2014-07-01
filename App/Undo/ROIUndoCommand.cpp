/*
    
    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#include "ROIUndoCommand.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ModifyROIUndoCommand::ModifyROIUndoCommand(const ViewManagerSPtr vm, const BinaryMaskSPtr<unsigned char> mask)
  : m_newROI     {nullptr}
  , m_viewManager{vm}
  , m_mask       {mask}
  {
    if(vm->currentROI() == nullptr)
      m_newROI = ROISPtr{new ROI{mask, mask->foregroundValue()}};
  }

  //-----------------------------------------------------------------------------
  ModifyROIUndoCommand::~ModifyROIUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ModifyROIUndoCommand::redo()
  {
    auto ROI = m_viewManager->currentROI();

    if(ROI == nullptr)
      m_viewManager->setCurrentROI(m_newROI);
    else
    {
      if(contains(ROI->bounds(), m_mask->bounds().bounds(), ROI->spacing()))
        ROI->draw(m_mask, m_mask->foregroundValue());
      else
      {
        ROI->resize(boundingBox(ROI->bounds(), m_mask->bounds().bounds()));
        ROI->draw(m_mask, m_mask->foregroundValue());
      }
    }
  }

  //-----------------------------------------------------------------------------
  void ModifyROIUndoCommand::undo()
  {
    if(m_newROI != nullptr)
      m_viewManager->setCurrentROI(nullptr);
    else
      m_viewManager->currentROI()->undo();
  }

  //-----------------------------------------------------------------------------
  ClearROIUndoCommand::ClearROIUndoCommand(const ViewManagerSPtr vm)
  : m_vm{vm}
  , m_roi{nullptr}
  {}

  //-----------------------------------------------------------------------------
  ClearROIUndoCommand::~ClearROIUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ClearROIUndoCommand::redo()
  {
    m_roi = m_vm->currentROI();
    m_vm->setCurrentROI(nullptr);
  }

  //-----------------------------------------------------------------------------
  void ClearROIUndoCommand::undo()
  {
    m_vm->setCurrentROI(m_roi);
    m_roi = nullptr;
  }
}
