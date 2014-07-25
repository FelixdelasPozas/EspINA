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


#ifndef COMPOSITIONTOOLBAR_H
#define COMPOSITIONTOOLBAR_H

#include <Core/Interfaces/IToolBar.h>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Model/EspinaModel.h>

class QUndoStack;

namespace ESPINA
{
  class CompositionToolBar
  : public IToolBar
  , public IFactoryExtension
  , public IFilterCreator
  {
    Q_OBJECT
    Q_INTERFACES
    (
      ESPINA::IFactoryExtension
      ESPINA::IFilterCreator
    )

  public:
    explicit CompositionToolBar(EspinaModel *model,
                                QUndoStack  *undoStack,
                                ViewManager *viewManager,
                                QWidget     *parent = 0);
    virtual ~CompositionToolBar();

    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual FilterSPtr createFilter(const QString              &filter,
                                    const Filter::NamedInputs  &inputs,
                                    const ModelItem::Arguments &args);

    virtual void resetToolbar(); // slot
    virtual void abortOperation() {};

  private slots:
    void createSegmentationFromComponents();
    void updateAvailableOperations();

  private:
    void initComposeTool();

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    QAction *m_compose;
  };

} // namespace ESPINA

#endif // COMPOSITIONTOOLBAR_H
