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

// EspINA
#include "NodesInformationDialog.h"
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/ModelItem.h>
#include <Core/EspinaTypes.h>

// Qt
#include <QFileDialog>

namespace EspINA
{

  class NodesFilter
  : public QSortFilterProxyModel
  {
      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
      {
        // We need to accept Segmentation Root
        if (!source_parent.isValid() && source_row == 3)
          return true;

        QModelIndex index = source_parent.child(source_row, 0);
        ModelItemPtr item = indexPtr(index);
        if (item == NULL)
          return false;

        if (SEGMENTATION != item->type())
          return false;

        Segmentation *seg = dynamic_cast<Segmentation *>(item);
        return seg->filter()->data().toString() == TubularSegmentationFilter::FILTER_TYPE;
      }
  };

  //------------------------------------------------------------------------
  NodesInformationDialog::NodesInformationDialog(ModelAdapter *model,
                                                 QUndoStack *undoStack,
                                                 ViewManager *vm,
                                                 TubularSegmentationFilter::Pointer filter,
                                                 QWidget *parent)
  : QDialog(parent)
  , m_undoStack(undoStack)
  , m_viewManager(vm)
  , m_model(model)
  , m_sort(new NodesFilter())
  , m_filter(filter)
  , m_lastWidget(NULL)
  {
    setWindowTitle("Spine Information");
    setObjectName("SpineInformationDialog");

    setupUi(this);
    QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
    m_writeDataToFile->setIcon(iconSave);
    connect(m_writeDataToFile, SIGNAL(clicked(bool)), this, SLOT(exportInformation()));

    m_sort->setSourceModel(m_model);
    m_sort->setDynamicSortFilter(true);
    m_sort->setSortRole(Qt::UserRole + 1);

    m_list->setModel(m_sort.get());
    m_list->setRootIndex(m_sort->mapFromSource(m_model->segmentationRoot()));

    connect(m_list, SIGNAL(clicked(QModelIndex)), this, SLOT(showSpineInformation(QModelIndex)));

    showSpineInformation(m_list->currentIndex());
  }

  //------------------------------------------------------------------------
  void NodesInformationDialog::exportInformation()
  {
    if (m_sort->rowCount() == 0)
      return;

    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setOption(QFileDialog::ShowDirsOnly);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite);

    if (fileDialog.exec() != QFileDialog::AcceptSave)
      return;

    QDir dir(fileDialog.selectedFiles().first());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    foreach(SegmentationSPtr seg, m_model->segmentations())if (true)
    {
      if (TubularSegmentationFilter::FILTER_TYPE == seg->filter()->data().toString())
      {
        QString file = dir.absoluteFilePath(seg->data().toString()).append(".txt");
        QFile spineFile(file);
        spineFile.open(QIODevice::WriteOnly| QIODevice::Text);

        QString separator = " ";
        QTextStream out(&spineFile);
        TubularSegmentationFilter *source = reinterpret_cast<TubularSegmentationFilter*>(seg->filter().get());
        foreach(QVector4D node, source->nodes())
        {
          out << node.x() << separator << node.y() << separator
          << node.z() << separator << node.w() << "\n";
        }
        spineFile.close();
      }
    }
    QApplication::restoreOverrideCursor();
  }

  //------------------------------------------------------------------------
  void NodesInformationDialog::showSpineInformation(QModelIndex index)
  {
    if (!index.isValid())
      return;

    ModelItemPtr item = indexPtr(m_sort->mapToSource(index));
    if (SEGMENTATION == item->type())
    {
      if (m_lastWidget)
        m_group->layout()->removeWidget(m_lastWidget);

      m_lastWidget = m_filter->filterInspector()->createWidget(m_undoStack, m_viewManager);

      m_group->layout()->addWidget(m_lastWidget);
    }
  }
}
