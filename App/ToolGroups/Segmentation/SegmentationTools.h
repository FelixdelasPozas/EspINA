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
#ifndef ESPINA_SEGMENTATION_TOOLS_H
#define ESPINA_SEGMENTATION_TOOLS_H

#include <Support/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>

#include "SeedGrowSegmentationTool.h"

#include <QAction>

class QUndoStack;

namespace EspINA
{
  /// Seed Growing Segmentation Plugin
  class SegmentationTools
  : public ToolGroup
  {
  public:
    SegmentationTools(ModelAdapterSPtr model,
                      ModelFactorySPtr factory,
                      ViewManagerSPtr  viewManager,
                      QUndoStack      *undoStack,
                      QWidget         *parent = nullptr);
    virtual ~SegmentationTools();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual ToolSList tools();

  private:
    SeedGrowSegmentationToolSPtr m_sgsTool;
  };

} // namespace EspINA

#endif// ESPINA_SEGMENTATION_TOOLS_H
