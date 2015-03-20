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
     * \param[in] model model adapter smart pointer.
     * \param[in] factory factory smart pointer.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] undoStack QUndoStack object raw pointer.
     * \param[in] parent QWidget raw pointer of the parent of this object.
     *
     */
    explicit RefineToolGroup(ModelAdapterSPtr model,
                          ModelFactorySPtr factory,
                          FilterDelegateFactorySPtr filterDelegateFactory,
                          ViewManagerSPtr  viewManager,
                          QUndoStack      *undoStack,
                          QWidget         *parent = nullptr);

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
    ManualEditionToolSPtr        m_manualEdition;
    SplitToolSPtr                m_split;
    MorphologicalEditionToolSPtr m_morphological;
    ModelFactorySPtr             m_factory;
    QUndoStack                  *m_undoStack;
    ModelAdapterSPtr             m_model;
    ViewManagerSPtr              m_viewManager;

    bool                         m_enabled;
    ContourWidget::ContourData   m_previousContour;
  };

} // namespace ESPINA

#endif // ESPINA_EDITION_TOOLS_H_
