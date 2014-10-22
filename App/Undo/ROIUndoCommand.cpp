/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "ROIUndoCommand.h"

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  AddROIUndoCommand::AddROIUndoCommand(ROIToolsGroup *toolsGroup, const BinaryMaskSPtr<unsigned char> mask)
  : m_newROI   {nullptr}
  , m_toolGroup{toolsGroup}
  , m_mask     {mask}
  {
    if(m_toolGroup->currentROI() == nullptr)
      m_newROI = ROISPtr{new ROI{mask}};
  }

  //-----------------------------------------------------------------------------
  AddROIUndoCommand::~AddROIUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void AddROIUndoCommand::redo()
  {
    auto ROI = m_toolGroup->currentROI();

    if(ROI == nullptr)
    {
      m_toolGroup->setCurrentROI(m_newROI);
    }
    else
    {
      expandAndDraw(ROI, m_mask);
    }
  }

  //-----------------------------------------------------------------------------
  void AddROIUndoCommand::undo()
  {
    if(m_newROI != nullptr)
    {
      m_toolGroup->setCurrentROI(nullptr);
    }
    else
    {
      m_toolGroup->currentROI()->undo();
    }
  }

  //-----------------------------------------------------------------------------
  ClearROIUndoCommand::ClearROIUndoCommand(ROIToolsGroup *toolsGroup)
  : m_toolGroup{toolsGroup}
  , m_roi      {nullptr}
  {}

  //-----------------------------------------------------------------------------
  ClearROIUndoCommand::~ClearROIUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ClearROIUndoCommand::redo()
  {
    m_roi = m_toolGroup->currentROI();
    m_toolGroup->setCurrentROI(nullptr);
  }

  //-----------------------------------------------------------------------------
  void ClearROIUndoCommand::undo()
  {
    m_toolGroup->setCurrentROI(m_roi);
    m_roi = nullptr;
  }
}
