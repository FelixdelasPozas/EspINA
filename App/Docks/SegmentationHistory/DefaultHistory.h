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

#ifndef ESPINA_DEFAULT_HISTORY_H
#define ESPINA_DEFAULT_HISTORY_H

#include <qt4/QtGui/QWidget>

#include <GUI/Model/SegmentationAdapter.h>

namespace ESPINA {

  namespace Ui
  {
    class DefaultHistory;
  }

  class DefaultHistory
  : public QWidget
  {
    Q_OBJECT
  public:
    explicit DefaultHistory(SegmentationAdapterPtr segmentation,
                            QWidget               *parent = 0,
                            Qt::WindowFlags        f = 0);

  private:
    Ui::DefaultHistory* m_gui;
  };

} // namespace ESPINA

#endif // ESPINA_DEFAULT_HISTORY_H
