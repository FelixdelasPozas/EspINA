/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXDIALOG_H_
#define APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXDIALOG_H_

// ESPINA
#include <GUI/Types.h>
#include <Support/Context.h>

// Qt
#include <QDialog>
#include <QItemSelection>

class QCloseEvent;
class QTableWidget;

namespace ESPINA
{
  /** class AdjacencyMatrixDialog
   * \brief Implements a dialog that shows the adjacency matrix of the segmentations.
   *
   */
  class AdjacencyMatrixDialog
  : public QDialog
  {
    public:
      /** \brief AdjacencyMatrixDialog class constructor.
       * \param[in] segmentations list of selected segmentations for the matrix or empty for all segmentations.
       * \param[in] model session model.
       *
       */
      explicit AdjacencyMatrixDialog(SegmentationAdapterList segmentations, Support::Context &context);

      /** \brief AdjacencyMatrixDialog class virtual destructor.
       *
       */
      virtual ~AdjacencyMatrixDialog()
      {}

    protected:
      virtual void closeEvent(QCloseEvent *event) override;
  };

} // namespace ESPINA

#endif // APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXDIALOG_H_
