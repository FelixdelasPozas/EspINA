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
#include <Dialogs/TabularReport/TabularReport.h>
#include <GUI/View/View3D.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Representations/Renderers/MeshRenderer.h>
#include <Support/Settings/EspinaSettings.h>

// EspINA

// Qt
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QDragEnterEvent>
#include <QStack>
#include <QHBoxLayout>

const QString SegmentationInspectorSettingsKey = QString("Segmentation Inspector Window Geometry");
const QString SegmentationInspectorSplitterKey = QString("Segmentation Inspector Splitter State");

using namespace EspINA;

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(SegmentationAdapterList segmentations,
                                             ModelAdapterSPtr        model,
                                             ModelFactorySPtr        factory,
                                             ViewManagerSPtr         viewManager,
                                             QUndoStack*             undoStack,
                                             QWidget*                parent,
                                             Qt::WindowFlags         flags)
: QWidget(parent, flags|Qt::WindowStaysOnTopHint)
, m_model(model)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
, m_view(new View3D(true))
, m_filterArea(new QScrollArea(this))
, m_tabularReport(new TabularReport(factory, viewManager))
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);

  for(auto segmentation : segmentations)
  {
    addSegmentation(segmentation);
    connect(segmentation, SIGNAL(modified(ItemAdapterPtr)),
            this,         SLOT(updateScene(ItemAdapterPtr)));
  }

  RendererSList renderers;
  renderers << RendererSPtr{new MeshRenderer()};
  // TODO: use view manager's
  m_view->setRenderers(renderers);
  m_view->setColorEngine(m_viewManager->colorEngine());
  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->resetCamera();
  m_view->updateView();

  SegmentationExtension::InfoTagList tags;
  tags << tr("Name") << tr("Taxonomy");
//   foreach(Segmentation::InformationExtension extension, m_model->factory()->segmentationExtensions())
//   {
//     if (extension->validTaxonomy(""))
//       tags << extension->availableInformations();
//   }

  m_filterArea->setWidget(new QWidget());
  m_filterArea->widget()->setMinimumWidth(250);
  m_filterArea->setMinimumWidth(250);

  QHBoxLayout *upperLayout = new QHBoxLayout();
  upperLayout->addWidget(m_view, 1,0);
  upperLayout->addWidget(m_filterArea, 0,0);

  m_tabularReport->setModel(m_model);
  m_tabularReport->setFilter(m_segmentations);
  m_tabularReport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_tabularReport->setVisible(true);
  m_tabularReport->setMinimumHeight(0);

  QHBoxLayout *lowerLayout = new QHBoxLayout();
  lowerLayout->insertWidget(0, m_tabularReport);

  m_upperWidget->setLayout(upperLayout);
  m_lowerWidget->setLayout(lowerLayout);
  m_splitter->setChildrenCollapsible(true);

//   connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection,bool)),
//           this, SLOT(updateSelection(ViewManager::Selection)));

  QSettings settings(CESVIMA, ESPINA);
  QByteArray geometry = settings.value(SegmentationInspectorSettingsKey, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
    restoreGeometry(geometry);

  QByteArray state = settings.value(SegmentationInspectorSplitterKey, QByteArray()).toByteArray();
  if (!state.isEmpty())
    m_splitter->restoreState(state);

  generateWindowTitle();
  updateSelection(m_viewManager->selection());
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  QSettings settings(CESVIMA, ESPINA);
  settings.setValue(SegmentationInspectorSettingsKey, this->saveGeometry());
  settings.setValue(SegmentationInspectorSplitterKey, m_splitter->saveState());
  settings.sync();

  QWidget::closeEvent(e);
  emit inspectorClosed(this);
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
  delete m_view;
  delete m_tabularReport;
}

//------------------------------------------------------------------------
void SegmentationInspector::updateScene(ItemAdapterPtr item)
{
  auto segmentation = segmentationPtr(item);
  m_view->updateRepresentation(segmentation);
  m_view->updateView();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateSelection(SelectionSPtr selection)
{
//   QWidget *prevWidget = m_filterArea->widget();
//   if (prevWidget)
//     delete prevWidget;
//
//   QWidget *noWidgetInspector = new QWidget();
//   noWidgetInspector->setLayout(new QVBoxLayout());
//   noWidgetInspector->layout()->setSpacing(10);
//   noWidgetInspector->setMaximumWidth(250);
//
//   QLabel *infoLabel = new QLabel(noWidgetInspector);
//
//   // channels could be part of the selection, we're only interested in selected segmentations
//   ViewManager::Selection clearedSelection;
//   foreach (PickableItemPtr item, selection)
//   if (item->type() == EspINA::SEGMENTATION)
//     clearedSelection << item;
//
//   if ((clearedSelection.size() == 1) && (EspINA::SEGMENTATION == clearedSelection.first()->type()) && m_segmentations.contains(segmentationPtr(clearedSelection.first())))
//   {
//     Filter::FilterInspectorPtr inspector = segmentationPtr(clearedSelection.first())->filter()->filterInspector();
//     if (inspector)
//     {
//       delete noWidgetInspector;
//       QWidget *inspectorwidget = inspector->createWidget(m_undoStack, m_viewManager);
//       inspectorwidget->setMaximumWidth(250);
//       m_filterArea->setWidget(inspectorwidget);
//       return;
//     }
//     else
//     {
//       infoLabel->setText(QLabel::tr("Segmentation cannot be modified."));
//     }
//   }
//   else
//   {
//     infoLabel->setText(QLabel::tr("No segmentation selected."));
//   }
//
//   infoLabel->setWordWrap(true);
//   infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
//   noWidgetInspector->layout()->addWidget(infoLabel);
//
//   QSpacerItem* spacer = new QSpacerItem(-1, -1, QSizePolicy::Minimum, QSizePolicy::Expanding);
//   noWidgetInspector->layout()->addItem(spacer);
//
//   m_filterArea->setWidget(noWidgetInspector);
}


//------------------------------------------------------------------------
void SegmentationInspector::addSegmentation(SegmentationAdapterPtr segmentation)
{
  if (m_segmentations.contains(segmentation))
    return;

  m_segmentations << segmentation;
  m_view->add(segmentation);

  auto channels = QueryAdapter::channels(segmentation);

  if(channels.isEmpty())
  {
    qDebug() << "FIXME: Channels shouldn't be empty" << __FILE__ << __LINE__;
    return;
  }

  if (channels.size() > 1)
  {
    qWarning() << "Tilinig is not supported yet";
  }
  else
  {
    addChannel(channels.first().get());
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::removeSegmentation(SegmentationAdapterPtr segmentation)
{
  if (!m_segmentations.contains(segmentation))
    return;

  m_segmentations.removeOne(segmentation);

  if (m_segmentations.size() == 0)
  {
    close();
    return;
  }

  m_view->remove(segmentation);

  ChannelAdapterSPtr channelToBeRemoved;
  auto channels = QueryAdapter::channels(segmentation);

  Q_ASSERT(!channels.isEmpty());

  if (channels.size() > 1)
  {
    qWarning() << "Tilinig is not supported yet";
  }
  else
  {
    // if there aren't any other segmentations that uses
    // this channel. we must remove it
    channelToBeRemoved = channels.first();

    bool stillUsed = false;
    for(auto remainingSegmenation : m_segmentations)
    {
      auto remainingChannels = QueryAdapter::channels(remainingSegmenation);
      for (auto remainingChannel : remainingChannels)
      {
        if (remainingChannel == channelToBeRemoved)
        {
          stillUsed = true;
        }
      }
    }

    if (!stillUsed)
    {
      removeChannel(channelToBeRemoved.get());
    }
  }

  m_view->updateView();

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::addChannel(ChannelAdapterPtr channel)
{
  if (m_channels.contains(channel))
    return;

  m_channels << channel;

  m_view->add(channel);
  m_view->updateView();

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::removeChannel(ChannelAdapterPtr channel)
{
  if (!m_channels.contains(channel))
    return;

  m_channels.removeOne(channel);

  m_view->remove(channel);
  m_view->updateView();

  generateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::generateWindowTitle()
{
  QString title(tr("Segmentation Inspector - "));

  for(auto segmentation : m_segmentations)
  {
    title += segmentation->data().toString();
    if(segmentation != m_segmentations.last())
      title += QString(" + ");
  }

  title += QString(" [");

  for(auto channel : m_channels)
  {
    title += channel->data().toString();
    if (channel != m_channels.last())
      title += QString(" + ");
  }
  title += QString("]");

  setWindowTitle(title);
}

//------------------------------------------------------------------------
void SegmentationInspector::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  updateSelection(m_viewManager->selection());
  m_tabularReport->updateSelection(m_viewManager->selection()->segmentations());
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
//   TODO + WARNING: this dropEvent() only works for Category (or "Type") layout
//   but right now that is the only layout with drag & drop enabled
  QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  QList<ItemData>         draggedItems;
  SegmentationAdapterList categorySegmentations;

  while (!stream.atEnd())
  {
    int row, col;
    QMap<int, QVariant> itemData;
    stream >> row >> col >> itemData;

    switch (ItemAdapter::type(itemData[TypeRole].toInt()))
    {
      case ItemAdapter::Type::SEGMENTATION:
      {
        draggedItems << itemData;
        break;
      }
      case ItemAdapter::Type::CATEGORY:
      {
        auto item     = reinterpret_cast<ItemAdapterPtr>(itemData[RawPointerRole].value<quintptr>());
        auto category = categoryPtr(item);

        for(auto segmentation : m_model->segmentations())
        {
          auto segmentationCategory = segmentation->category().get();
          while(segmentationCategory != m_model->classification()->root().get())
          {
            if (segmentationCategory == category)
            {
              categorySegmentations << segmentation.get();
              break;
            }

            segmentationCategory = segmentationCategory->parent();
          }
        }
        break;
      }
      default:
        break;
    }
  }

  for(auto data : draggedItems)
  {
    auto item = reinterpret_cast<ItemAdapterPtr>(data[RawPointerRole].value<quintptr>());
    auto segmentation = segmentationPtr(item);

    if (!m_segmentations.contains(segmentation))
    {
      addSegmentation(segmentationPtr(item));
    }
  }

  for(auto segmentation : categorySegmentations)
  {
    addSegmentation(segmentation);
  }

  m_tabularReport->setFilter(m_segmentations);

  updateSelection(m_viewManager->selection());
  m_tabularReport->updateSelection(m_viewManager->selection()->segmentations());

  m_view->updateView();
  m_view->resetCamera();

  event->acceptProposedAction();
}
