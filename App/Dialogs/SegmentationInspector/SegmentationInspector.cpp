/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#include "SegmentationInspector.h"

// EspINA
#include <Core/Model/ModelItem.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/Filter.h>
#include <GUI/QtWidget/VolumeView.h>
#include <Core/EspinaSettings.h>

// Qt
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QDebug>
#include <QDragEnterEvent>

const QString SegmentationInspectorSettingsKey = QString("Segmentation Inspector Window Geometry");

using namespace EspINA;

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(SegmentationList segmentations,
                                             EspinaModel     *model,
                                             QUndoStack      *undoStack,
                                             ViewManager     *vm,
                                             QWidget         *parent,
                                             Qt::WindowFlags  flags)
: QWidget(parent, flags|Qt::WindowStaysOnTopHint)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_info(new InformationProxy())
, m_sort(new QSortFilterProxyModel())
, m_view(new VolumeView(model->factory(), vm, true))
{
  m_view->setViewType(VOLUME);

  setupUi(this);
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setAcceptDrops(true);

  QString regExpression;
  foreach(SegmentationPtr seg, segmentations)
  {
    addSegmentation(seg);
    connect(seg, SIGNAL(modified(ModelItemPtr)),
            this, SLOT(updateScene(ModelItemPtr)));

    regExpression += "^"+seg->data().toString()+"$";
    if(seg != segmentations.last())
      regExpression += "|";
  }

  m_view->resetCamera();
  m_view->updateView();
  horizontalLayout->insertWidget(0, m_view);

  Segmentation::InfoTagList tags;
  tags << tr("Name") << tr("Taxonomy");
  foreach(Segmentation::InformationExtension extension, m_model->factory()->segmentationExtensions())
  {
    tags << extension->availableInformations();
  }

  m_info->setQuery(tags);
  m_info->setSourceModel(m_model);
  m_sort->setSourceModel(m_info.data());
  m_sort->setFilterRegExp(regExpression);
  m_sort->setDynamicSortFilter(true);
  m_dataView->setModel(m_sort.data());
  m_dataView->setSortingEnabled(true);// Needed to update values when segmentation is modified
  m_dataView->sortByColumn(0, Qt::AscendingOrder);

  connect(m_dataView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(updateSelection(QItemSelection,QItemSelection)));
  connect(m_dataView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(centerViewOn(QModelIndex)));

  QSettings settings(CESVIMA, ESPINA);
  QByteArray geometry = settings.value(SegmentationInspectorSettingsKey, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
    restoreGeometry(geometry);

  updateSelection();
  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue(SegmentationInspectorSettingsKey, this->saveGeometry());
  settings.sync();

  QWidget::closeEvent(e);
  emit inspectorClosed(this);
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
  delete m_view;
}

//------------------------------------------------------------------------
void SegmentationInspector::updateScene(ModelItemPtr item)
{
  SegmentationPtr seg = segmentationPtr(item);
  m_view->updateSegmentation(seg);
  m_view->updateView();
}

//------------------------------------------------------------------------
void SegmentationInspector::addSegmentation(SegmentationPtr segmentation)
{
  if (m_segmentations.contains(segmentation))
    return;

  m_segmentations << segmentation;
  m_view->addSegmentation(segmentation);
  m_view->updateView();

  addChannel(segmentation->channel().data());

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::removeSegmentation(SegmentationPtr segmentation)
{
  if (!m_segmentations.contains(segmentation))
    return;

  m_segmentations.removeOne(segmentation);

  if (m_segmentations.size() == 0)
  {
    close();
    return;
  }

  m_view->removeSegmentation(segmentation);

  // if there aren't any other segmentations that uses this one's
  // channel, we must remove the channel
  ChannelSPtr channel = segmentation->channel();
  bool channelFound = false;
  foreach(SegmentationPtr seg, m_segmentations)
    if (seg->channel() == channel)
      channelFound = true;

  if (!channelFound)
    removeChannel(channel.data());

  m_view->updateView();
  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::addChannel(ChannelPtr channel)
{
  if (m_channels.contains(channel))
    return;

  m_channels << channel;
  m_view->addChannel(channel);
  m_view->updateView();

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::removeChannel(ChannelPtr channel)
{
  if (!m_channels.contains(channel))
    return;

  m_channels.removeOne(channel);

  m_view->removeChannel(channel);
  m_view->updateView();

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateSelection(QItemSelection selected, QItemSelection deselected)
{
  QWidget *prevWidget = m_filterInspector->widget();
  if (prevWidget)
    delete prevWidget;

  ModelItemPtr item(NULL);
  if (!m_dataView->selectionModel()->selectedIndexes().isEmpty())
  {
    item = indexPtr(m_info->mapToSource(m_sort->mapToSource(m_dataView->selectionModel()->selectedIndexes().first())));

    if (EspINA::SEGMENTATION == item->type() && m_segmentations.contains(segmentationPtr(item)))
    {
      Filter::FilterInspectorPtr inspector = segmentationPtr(item)->filter()->filterInspector();
      if (inspector)
      {
        QWidget *widget = inspector->createWidget(m_undoStack, m_viewManager);
        m_filterInspector->setWidget(widget);
        m_filterInspector->setMinimumWidth((widget->minimumSize().width() < 250) ?  250 : widget->minimumSize().width());
        return;
      }
    }
  }

  QWidget *noWidgetInspector = new QWidget();
  noWidgetInspector->setLayout(new QVBoxLayout());
  noWidgetInspector->layout()->setSpacing(10);

  QLabel *infoLabel = new QLabel(noWidgetInspector);
  if (item != NULL)
    infoLabel->setText(QLabel::tr("This filter doesn't have configurable options."));
  else
    infoLabel->setText(QLabel::tr("No segmentation selected."));
  infoLabel->setWordWrap(true);
  infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
  noWidgetInspector->layout()->addWidget(infoLabel);

  QSpacerItem* spacer = new QSpacerItem(-1, -1, QSizePolicy::Minimum, QSizePolicy::Expanding);
  noWidgetInspector->layout()->addItem(spacer);

  m_filterInspector->setWidget(noWidgetInspector);
  m_filterInspector->setMinimumWidth(250);
}

//------------------------------------------------------------------------
void SegmentationInspector::generateWindowTitle()
{
  QString title("Segmentation Inspector - ");
  foreach(SegmentationPtr segmentation, m_segmentations)
  {
    title += segmentation->data().toString();
    if(segmentation != m_segmentations.last())
      title += QString(" + ");
  }
  title += QString(" [");
  foreach(ChannelPtr channel, m_channels)
  {
    title += channel->data().toString();
    if (channel != m_channels.last())
      title += QString(" + ");
  }
  title += QString("]");

  setWindowTitle(title);
}

//------------------------------------------------------------------------
void SegmentationInspector::centerViewOn(QModelIndex index)
{
  if (!index.isValid())
    return;

  QModelIndex modelIndex = m_info->mapToSource(m_sort->mapToSource(index));

  ModelItemPtr item = indexPtr(modelIndex);
  if (item->type() != SEGMENTATION)
    return;

  SegmentationPtr segmentation = segmentationPtr(item);
  double bounds[6];
  segmentation->volume()->bounds(bounds);
  Nm center[3] = { (bounds[0] + bounds[1]) / 2.0, (bounds[2] + bounds[3]) / 2.0, (bounds[4] + bounds[5]) / 2.0 };
  m_view->centerViewOn(center, false);
}

//------------------------------------------------------------------------
void SegmentationInspector::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    event->acceptProposedAction();
}

//------------------------------------------------------------------------
void SegmentationInspector::dragMoveEvent(QDragMoveEvent *event)
{
  // TODO: show the drop area of the drag before entering the dialog
  // (probably needs a signal from origin widget to all possible target widgets)
  if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
    event->accept();
  else
    event->ignore();
}

typedef QMap<int, QVariant> ItemData;

//------------------------------------------------------------------------
void SegmentationInspector::dropEvent(QDropEvent *event)
{
  setGraphicsEffect(NULL);
  QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  QList<ItemData> draggedItems;
  SegmentationList taxSegmentations;

  while (!stream.atEnd())
  {
    int row, col;
    QMap<int, QVariant> itemData;
    stream >> row >> col >> itemData;

    if (itemData[TypeRole].toInt() == EspINA::SEGMENTATION)
      draggedItems << itemData;

    if (itemData[TypeRole].toInt() == EspINA::TAXONOMY)
    {
      ModelItemPtr item = reinterpret_cast<ModelItemPtr>(itemData[RawPointerRole].value<quintptr>());
      TaxonomyElementPtr taxonomy = taxonomyElementPtr(item);
      foreach(SegmentationSPtr seg, m_model->segmentations())
      {
        TaxonomyElementPtr segTax = seg->taxonomy().data();
        while(segTax != m_model->taxonomy()->root().data())
        {
          if (segTax == taxonomy)
          {
            taxSegmentations << seg.data();
            break;
          }

          segTax = segTax->parent();
        }
      }
    }
  }

  foreach(ItemData data, draggedItems)
  {
    ModelItemPtr item = reinterpret_cast<ModelItemPtr>(data[RawPointerRole].value<quintptr>());
    SegmentationPtr segmentation = segmentationPtr(item);
    if (m_segmentations.contains(segmentation))
      continue;

    addSegmentation(segmentationPtr(item));
  }

  foreach(SegmentationPtr seg, taxSegmentations)
    addSegmentation(seg);

  // resetting the sort is faster than modifying the regular expression
  QString regExpression;
  foreach(SegmentationPtr seg, m_segmentations)
  {
    regExpression += "^"+seg->data().toString()+"$";
    if(seg != m_segmentations.last())
      regExpression += "|";
  }
  m_sort->setFilterRegExp(regExpression);
  m_dataView->setModel(m_sort.data());

  m_view->resetCamera();

  event->acceptProposedAction();
}
