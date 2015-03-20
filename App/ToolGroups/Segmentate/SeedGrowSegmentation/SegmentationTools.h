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

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include "SeedGrowSegmentationTool.h"
#include <ToolGroups/ToolGroup.h>

// Qt
#include <QAction>

class QUndoStack;

namespace ESPINA
{

  class SeedGrowSegmentationSettings;

  class SegmentationTools
  : public ToolGroup
  {
  public:
		/** \brief SegmentationTools class constructor.
		 * \param[in] settings, raw pointer to a SeedGrowSegmentationSettings object.
		 * \param[in] model, model adapter smart pointer.
		 * \param[in] factory, factory smart pointer.
		 * \param[in] undoStack, QUndoStack raw pointer.
		 * \param[in] parent, QWidget raw pointer of the parent of this object.
		 *
		 */
    SegmentationTools(SeedGrowSegmentationSettings* settings,
                      ModelAdapterSPtr              model,
                      ModelFactorySPtr              factory,
                      FilterDelegateFactorySPtr     filterDelegateFactory,
                      ViewManagerSPtr               viewManager,
                      QUndoStack                   *undoStack,
                      QWidget                      *parent = nullptr);

    /** \brief SegmentationTools class virtual destructor.
     *
     */
    virtual ~SegmentationTools();

    /** \brief Implements ToolGroup::setEnabled().
     *
     */
    virtual void setEnabled(bool value);

    /** \brief Implements ToolGroup::enabled().
     *
     */
    virtual bool enabled() const;

    /** \brief Implements ToolGroup::tools().
     *
     */
    virtual ToolSList tools();

  private:
    SeedGrowSegmentationToolSPtr m_sgsTool;
  };

} // namespace ESPINA

#endif// ESPINA_SEGMENTATION_TOOLS_H
