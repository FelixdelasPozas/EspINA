/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2013  <copyright holder> <email>
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


#ifndef ESPINA_CHANGE_CATEGORY_COMMAND_H
#define ESPINA_CHANGE_CATEGORY_COMMAND_H

#include "EspinaUndo_Export.h"

// EspINA
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>
#include <QMap>

namespace EspINA
{
  class ModelAdapter;
  class ViewManager;

  class EspinaUndo_EXPORT ChangeCategoryCommand
  : public QUndoCommand
  {
  public:
    explicit ChangeCategoryCommand(SegmentationAdapterSList segmentations,
                                   CategoryAdapterSPtr     category,
                                   ModelAdapterSPtr        model,
                                   ViewManagerSPtr         viewManager,
                                   QUndoCommand           *parent = nullptr);
    virtual ~ChangeCategoryCommand();

    virtual void redo();
    virtual void undo();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    CategoryAdapterSPtr m_category;
    QMap<SegmentationAdapterSPtr, CategoryAdapterSPtr> m_oldCategories;
  };

} // namespace EspINA

#endif // ESPINA_CHANGE_CATEGORY_COMMAND_H
