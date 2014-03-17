/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
#ifndef ESPINA_VOI_TOOLS_H
#define ESPINA_VOI_TOOLS_H

#include <Support/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>
#include "BrushVOITool.h"
#include "OrtogonalVOITool.h"
#include "CleanVOITool.h"

#include <QAction>

class QUndoStack;

namespace EspINA
{
  /// Seed Growing Segmentation Plugin
  class VolumeOfInterestTools
  : public ToolGroup
  {
  public:
    VolumeOfInterestTools(ModelAdapterSPtr model,
                      ModelFactorySPtr factory,
                      ViewManagerSPtr  viewManager,
                      QUndoStack      *undoStack,
                      QWidget         *parent = nullptr);
    virtual ~VolumeOfInterestTools();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual ToolSList tools();

  private:
    BrushVOIToolSPtr     m_brushVOITool;
    OrtogonalVOIToolSPtr m_ortogonalVOITool;
    CleanVOIToolSPtr     m_cleanVOITool;
  };

} // namespace EspINA

#endif// ESPINA_VOI_TOOLS_H
