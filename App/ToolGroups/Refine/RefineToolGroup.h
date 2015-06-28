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

#ifndef ESPINA_SUPPORT_REFINE_TOOL_H
#define ESPINA_SUPPORT_REFINE_TOOL_H

// ESPINA
#include <ToolGroups/ToolGroup.h>

#include <Support/Factory/FilterRefinerRegister.h>

class QUndoStack;

namespace ESPINA
{
  class RefineToolGroup
  : public ToolGroup
  , private Support::WithContext
  {
    Q_OBJECT
  public:
    /** \brief EditionTools class constructor.
     * \param[in] filterDelegateFactory factory
     * \param[in] context ESPINA context
     *
     */
    explicit RefineToolGroup(Support::FilterRefinerRegister &filterRefiners,
                             Support::Context               &context);

    virtual ~RefineToolGroup();

  private:
    void initManualEditionTool();

    void initSplitTool();

    void initCODETools();

    void initFillHolesTool();

    void initImageLogicTools();

  private slots:
    /** \brief Deletes a segmentation from the model if all its voxels have been erased.
     *
     */
    void onVoxelDeletion(ViewItemAdapterPtr item);
  };

} // namespace ESPINA

#endif // ESPINA_SUPPORT_REFINE_TOOL_H
