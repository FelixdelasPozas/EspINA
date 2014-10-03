/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
#define ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H

#include <qt4/QtGui/QWidget>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <Support/ViewManager.h>

class QUndoStack;

namespace ESPINA {

  namespace Ui
  {
    class SeedGrowSegmentationHistoryWidget;
  }

  class SeedGrowSegmentationHistoryWidget
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit SeedGrowSegmentationHistoryWidget(std::shared_ptr<SeedGrowSegmentationFilter> filter,
                                               ViewManagerSPtr             viewManager,
                                               QUndoStack                 *undoStack,
                                               QWidget                    *parent = 0,
                                               Qt::WindowFlags             f = 0);
  private slots:
    void modifyFilter();

  private:
    Ui::SeedGrowSegmentationHistoryWidget      *m_gui;
    std::shared_ptr<SeedGrowSegmentationFilter> m_filter;
    ViewManagerSPtr m_viewManager;
    QUndoStack     *m_undoStack;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
