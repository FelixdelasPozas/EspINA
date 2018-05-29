/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_ADD_CATEGORY_COMMAND_H
#define ESPINA_ADD_CATEGORY_COMMAND_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  /** \class AddCategoryCommand
   * \brief Undo command for inserting a category in the EspINA model adapter.
   *
   */
  class EspinaUndo_EXPORT AddCategoryCommand
  : public QUndoCommand
  {
  public:
    /** \brief AddCategoryCommand class constructor.
     * \param[in] parentCategory parent category adapter.
     * \param[in] category to add.
     * \param[in] model espina model
     * \param[in] parent QUndoCommand
     *
     */
    explicit AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                CategoryAdapterSPtr category,
                                ModelAdapterSPtr    model,
                                QUndoCommand*       parent = nullptr);

    /** \brief AddCategoryCommand class constructor.
     * \param[in] parentCategory parent category adapter.
     * \param[in] name of the new category.
     * \param[in] model espina model
     * \param[in] color the new category.
     * \param[in] parent QUndoCommand.
     *
     */
    explicit AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                const QString&      name,
                                ModelAdapterSPtr    model,
                                QColor              color,
                                QUndoCommand*       parent = nullptr);

    /** \brief AddCategoryCommand class constructor.
     * \param[in] parentCategory parent category adapter.
     * \param[in] name of the new category.
     * \param[in] model espina model
     * \param[in] color the new category.
     * \param[in] parent QUndoCommand.
     *
     */
    explicit AddCategoryCommand(CategoryAdapterSPtr parentCategory,
                                const QString&      name,
                                ModelAdapterSPtr    model,
                                QColor              color,
                                Vector3<long long>  roi,
                                QUndoCommand*       parent = nullptr);

    /** \brief AddCategoryCommand class virtual destructor.
     *
     */
    virtual ~AddCategoryCommand();

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    ModelAdapterSPtr    m_model;          /** model that contains the classification.          */
    QString             m_name;           /** new category name, if m_category is nullptr.     */
    QColor              m_color;          /** new category color, if m_category is nullptr.    */
    CategoryAdapterSPtr m_category;       /** category to add to the model, or nullptr.        */
    CategoryAdapterSPtr m_parentCategory; /** parent category of the one to add, nver null.    */
    Vector3<long long>  m_roiSize;        /** new category roi size, if m_category is nullptr. */
  };
} // ESPINA

#endif // ESPINA_ADD_CATEGORY_COMMAND_H
