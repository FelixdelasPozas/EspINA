/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_HISTORY_DOCK_H
#define ESPINA_HISTORY_DOCK_H

#include <Support/Widgets/DockWidget.h>
#include <Support/Factory/FilterDelegateFactory.h>

class QUndoStack;

namespace ESPINA
{
  class HistoryDock
  : public DockWidget
  {
    Q_OBJECT
  public:
    explicit HistoryDock(ModelAdapterSPtr          model,
                         ModelFactorySPtr          factory,
                         FilterDelegateFactorySPtr delegatFactory,
                         ViewManagerSPtr           viewManager,
                         QUndoStack               *undoStack,
                         QWidget                  *parent = 0);
    virtual ~HistoryDock();

    virtual void showEvent(QShowEvent* e);

    virtual void reset(); // slot

  protected slots:
    void updateDock();

  private:
    ModelAdapterSPtr          m_model;
    ModelFactorySPtr          m_factory;
    FilterDelegateFactorySPtr m_delegateFactory;
    ViewManagerSPtr           m_viewManager;
    QUndoStack               *m_undoStack;

    FilterSPtr             m_filter;
    SegmentationAdapterPtr m_segmentation;
  };

} // namespace ESPINA

#endif // ESPINA_HISTORY_DOCK_H
