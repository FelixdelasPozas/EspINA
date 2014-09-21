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

#ifndef ESPINA_CHANGE_CATEGORY_COMMAND_H
#define ESPINA_CHANGE_CATEGORY_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>
#include <QMap>

namespace ESPINA
{
  class ModelAdapter;
  class ViewManager;

  class EspinaUndo_EXPORT ChangeCategoryCommand
  : public QUndoCommand
  {
  public:
  	/** \brief ChangeCategoryCommand class constructor.
  	 * \param[in] segmentations, list of segmentation adapter raw pointers.
  	 * \param[in] category, raw pointer of the new category adapter.
  	 * \param[in] model, smart pointer of the model containig the segmentations the category.
  	 * \param[in] viewManager, view manager smart pointer.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit ChangeCategoryCommand(SegmentationAdapterList segmentations,
                                   CategoryAdapterPtr      category,
                                   ModelAdapterSPtr        model,
                                   ViewManagerSPtr         viewManager,
                                   QUndoCommand*           parent = nullptr);

    /** \brief ChangeCategoryCommand class virtual destructor.
     *
     */
    virtual ~ChangeCategoryCommand();

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    CategoryAdapterSPtr m_category;
    QMap<SegmentationAdapterSPtr, CategoryAdapterSPtr> m_oldCategories;
  };

} // namespace ESPINA

#endif // ESPINA_CHANGE_CATEGORY_COMMAND_H
