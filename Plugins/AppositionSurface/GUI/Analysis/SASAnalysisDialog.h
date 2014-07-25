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

#ifndef ESPINA_SAS_ANALYSIS_DIALOG_H
#define ESPINA_SAS_ANALYSIS_DIALOG_H

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QDialog>
#include <QUndoStack>

namespace ESPINA
{
  class SASAnalysisDialog
  : public QDialog
  {
  public:
    explicit SASAnalysisDialog(SegmentationAdapterList segmentations,
                               ModelAdapterSPtr        model,
                               QUndoStack             *undoStack,
                               ModelFactorySPtr        factory,
                               ViewManagerSPtr         viewManager,
                               QWidget                *parent);
  protected:
    virtual void closeEvent(QCloseEvent* event);
  };
}

#endif // ESPINA_SAS_ANALYSIS_DIALOG_H
