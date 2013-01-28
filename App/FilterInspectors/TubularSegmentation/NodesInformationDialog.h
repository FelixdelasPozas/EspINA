/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef SPINEINFORMATIONDIALOG_H
#define SPINEINFORMATIONDIALOG_H

// EspINA
#include "ui_NodesInformationDialog.h"
#include <Filters/TubularSegmentationFilter.h>

// Qt
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QVector4D>

class QUndoStack;

namespace EspINA
{
  class ViewManager;
  class ITool;
  class EspinaModel;

  /// Display Information of all segmentations that have been created
  class NodesInformationDialog
  : public QDialog
  , Ui::NodesInformationDialog
  {
    Q_OBJECT
    public:
      explicit NodesInformationDialog(EspinaModel *model,
                                      QUndoStack *undoStack,
                                      ViewManager *vm,
                                      TubularSegmentationFilter::Pointer filter,
                                      QWidget *parent=0);

    protected slots:
      void exportInformation();
      void showSpineInformation(QModelIndex index);

    private:
      QUndoStack                           *m_undoStack;
      ViewManager                          *m_viewManager;
      EspinaModel                          *m_model;
      QSharedPointer<QSortFilterProxyModel> m_sort;
      TubularSegmentationFilter::Pointer    m_filter;
      QWidget                              *m_lastWidget;
      ITool                                *tubularTool;
  };
}
#endif // SPINEINFORMATIONDIALOG_H
