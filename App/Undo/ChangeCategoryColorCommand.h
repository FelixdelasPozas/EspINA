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
#include <Core/EspinaTypes.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/RepresentationInvalidator.h>

// Qt
#include <QUndoStack>

using namespace ESPINA::GUI::View;

namespace ESPINA
{
  class ChangeCategoryColorCommand
  : public QUndoCommand
  {
    public:
      ChangeCategoryColorCommand(ModelAdapterSPtr model,
                                 RepresentationInvalidator &invalidator,
                                 CategoryAdapterPtr category,
                                 Hue hueValue);

      virtual ~ChangeCategoryColorCommand();
      
      virtual void redo() override;
      virtual void undo() override;

    private:
      void invalidateDependentSegmentations() const;

      ModelAdapterSPtr           m_model;
      RepresentationInvalidator &m_invalidator;
      CategoryAdapterPtr         m_category;
      Hue                        m_hueValue;
  };
    
} // namespace ESPINA

#endif // APP_UNDO_CHANGECATEGORYCOLORCOMMAND_H_
