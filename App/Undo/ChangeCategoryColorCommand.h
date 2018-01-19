/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_UNDO_CHANGECATEGORYCOLORCOMMAND_H_
#define APP_UNDO_CHANGECATEGORYCOLORCOMMAND_H_

// ESPINA
#include <Core/Types.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  /** \class ChangeCategoryColorCommand
   * \brief Undo command to provide undo/redo to a change of color of a category.
   *
   */
  class ChangeCategoryColorCommand
  : public QUndoCommand
  {
    public:
      /** \brief ChangeCategoryColorCommand class constructor.
       * \param[in] model model of the analysis.
       * \param[in] viewState current state of the views.
       * \param[in] category category to change color.
       * \param[in] hueValue new color hue value.
       *
       */
      ChangeCategoryColorCommand(ModelAdapterSPtr model,
                                 GUI::View::ViewState &viewState,
                                 CategoryAdapterPtr category,
                                 Hue hueValue);

      /** \brief ChangeCategoryColorCommand class virtual destructor.
       *
       */
      virtual ~ChangeCategoryColorCommand()
      {}

      virtual void redo() override;
      virtual void undo() override;

    private:
      /** \brief Helper method to invalidate the representations of dependent segmentations.
       *
       */
      void invalidateDependentSegmentations() const;

      ModelAdapterSPtr      m_model;     /** model of the analysis containing the category. */
      GUI::View::ViewState &m_viewState; /** current state of the views.                    */
      CategoryAdapterPtr    m_category;  /** category to change color.                      */
      Hue                   m_hueValue;  /** hue value of the color to undo/redo.           */
  };
    
} // namespace ESPINA

#endif // APP_UNDO_CHANGECATEGORYCOLORCOMMAND_H_
