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

#ifndef ESPINA_LEVEL_SELECTOR_DIALOG_H
#define ESPINA_LEVEL_SELECTOR_DIALOG_H

#include "Support/EspinaSupport_Export.h"

#include <QDialog>
#include <ui_LevelSelectorDialog.h>
#include <Coordinator.h>

namespace ESPINA
{
  class EspinaSupport_EXPORT LevelSelectorDialog
  : public QDialog
  , private Ui::LevelSelectorDialog
  {
   Q_OBJECT

  public:
    explicit LevelSelectorDialog(const std::vector<Metadona::Level>& levels,
                                 QWidget*                            parent = 0,
                                 Qt::WindowFlags                     f = 0);
    virtual ~LevelSelectorDialog();

    Metadona::Id selectedLevel() const
    { return m_selectedLevel; }

  private slots:
    void selectLevel();

    void stopSelection();

  private:
    Metadona::Id m_selectedLevel;
  };

}

#endif // ESPINA_LEVEL_SELECTOR_DIALOG_H
