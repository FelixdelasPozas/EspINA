/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
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


#ifndef COMPOSITIONTOOLBAR_H
#define COMPOSITIONTOOLBAR_H

#include <QToolBar>
#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>

class QUndoStack;

namespace EspINA
{
  class CompositionToolBar
  : public QToolBar
  , public IFactoryExtension
  , public IFilterCreator
  {
    Q_OBJECT
    Q_INTERFACES(EspINA::IFactoryExtension EspINA::IFilterCreator)

  public:
    explicit CompositionToolBar(EspinaModelPtr model,
                                QUndoStack    *undoStack,
                                ViewManager   *viewManager,
                                QWidget       *parent = 0);
    virtual ~CompositionToolBar();

    virtual void initFactoryExtension(EspinaFactoryPtr factory);

    virtual FilterPtr createFilter(const QString              &filter,
                                   const Filter::NamedInputs  &inputs,
                                   const ModelItem::Arguments &args);

  private slots:
    void createSegmentationFromComponents();
    void updateAvailableOperations();

  private:
    void initComposeTool();

  private:
    EspinaModelPtr m_model;
    QUndoStack    *m_undoStack;
    ViewManager   *m_viewManager;

    QAction *m_compose;
  };

} // namespace EspINA

#endif // COMPOSITIONTOOLBAR_H
