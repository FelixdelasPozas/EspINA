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


#ifndef CHANGETAXONOMYCOMMAND_H
#define CHANGETAXONOMYCOMMAND_H

#include "EspinaUndo_Export.h"

// EspINA
#include <Core/Model/Taxonomy.h>
#include <Core/Model/Segmentation.h>

// Qt
#include <QUndoStack>
#include <QMap>

namespace EspINA
{
  class EspinaModel;
  class ViewManager;

  class EspinaUndo_EXPORT ChangeTaxonomyCommand
  : public QUndoCommand
  {
  public:
    explicit ChangeTaxonomyCommand(SegmentationList   segmentations,
                                   TaxonomyElementPtr taxonomy,
                                   EspinaModel       *model,
                                   ViewManager       *viewManager,
                                   QUndoCommand      *parent = NULL);
    virtual ~ChangeTaxonomyCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    TaxonomyElementSPtr m_taxonomy;
    QMap<SegmentationSPtr, TaxonomyElementSPtr> m_oldTaxonomies;
  };

} // namespace EspINA

#endif // CHANGETAXONOMYCOMMAND_H
