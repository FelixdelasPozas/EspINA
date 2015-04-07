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

#ifndef ESPINA_EDITION_TOOLS_H_
#define ESPINA_EDITION_TOOLS_H_

// ESPINA
#include "ManualEditionTool.h"
#include "SplitTool.h"
#include "MorphologicalEditionTool.h"
#include <ToolGroups/ToolGroup.h>
#include <Core/Factory/FilterFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/Selection.h>
#include <GUI/ModelFactory.h>
#include <GUI/View/Widgets/Contour/ContourWidget.h>

class QUndoStack;

namespace ESPINA
{
  class RefineToolGroup
  : public ToolGroup
  {
    Q_OBJECT
  public:
    /** \brief EditionTools class constructor.
     * \param[in] context ESPIAN context
     * \param[in] filterDelegateFactory factory
     *
     */
    explicit RefineToolGroup(FilterDelegateFactorySPtr filterDelegateFactory,
                             const Support::Context   &context);

    virtual ~RefineToolGroup();

  public slots:
    /** \brief Updates the tools based on current selection.
     *
     */
    void enableCurrentSelectionActions();

  private slots:
    /** \brief Deletes a segmentation from the model if all its voxels have been erased.
     *
     */
    void onVoxelDeletion(ViewItemAdapterPtr item);

  private:
    const Support::Context      &m_context;

    ManualEditionToolSPtr        m_manualEdition;
    SplitToolSPtr                m_split;
    MorphologicalEditionToolSPtr m_morphological;

    bool                         m_enabled;
    ContourWidget::ContourData   m_previousContour;
  };

} // namespace ESPINA

#endif // ESPINA_EDITION_TOOLS_H_
