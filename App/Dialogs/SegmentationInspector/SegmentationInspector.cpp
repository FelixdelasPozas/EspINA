/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// ESPINA
#include "SegmentationInspector.h"
#include <Docks/SegmentationHistory/EmptyHistory.h>
#include <Docks/SegmentationHistory/DefaultHistory.h>
#include <Support/Widgets/TabularReport.h>
#include <GUI/View/View3D.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Representations/Renderers/MeshRenderer.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/FilterHistory.h>

// Qt
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QDragEnterEvent>
#include <QStack>
#include <QHBoxLayout>
#include <QDebug>

const QString SegmentationInspectorSettingsKey = QString("Segmentation Inspector Window Geometry");
const QString SegmentationInspectorSplitterKey = QString("Segmentation Inspector Splitter State");
const QString RENDERERS                        = QString("DefaultView::renderers");

using namespace ESPINA;

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(SegmentationAdapterList segmentations,
                                             ModelAdapterSPtr        model,
                                             ModelFactorySPtr        factory,
                                             ViewManagerSPtr         viewManager,
                                             QUndoStack*             undoStack,
                                             QWidget*                parent,
                                             Qt::WindowFlags         flags)
: QWidget        {parent, flags|Qt::WindowStaysOnTopHint}
, m_model        {model}
, m_viewManager  {viewManager}
, m_undoStack    {undoStack}
, m_view         {new View3D(true)}
, m_tabularReport{new TabularReport(factory, viewManager)}
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

  ESPINA_SETTINGS(settings);

  QStringList defaultRenderers;
  if (!settings.contains(RENDERERS) || !settings.value(RENDERERS).isValid())
  {
    defaultRenderers << m_viewManager->renderers(ESPINA::RendererType::RENDERER_VIEW3D);

    settings.setValue(RENDERERS, defaultRenderers);
  }

  defaultRenderers = settings.value(RENDERERS).toStringList();
  RendererSList renderers;

  for(auto name: defaultRenderers)
  {
    if(m_viewManager->renderers(RendererType::RENDERER_VIEW3D).contains(name))
    {
      renderers << m_viewManager->cloneRenderer(name);
    }
  }

  m_view->setRenderers(renderers);
  m_view->setColorEngine(m_viewManager->colorEngine());
  m_view->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  m_view->setMinimumWidth(250);
  m_view->resetCamera();
  m_view->updateView();

  QVBoxLayout *viewportLayout = new QVBoxLayout();
  viewportLayout->addWidget(m_view);
  viewportLayout->setSizeConstraint(QLayout::SetMinimumSize);

//   m_historyScrollArea->widget()->setMinimumWidth(250);
//   m_historyScrollArea->setMinimumWidth(150);

//   QVBoxLayout *historyLayout = new QVBoxLayout();
//   historyLayout->addWidget(m_historyScrollArea);

  m_tabularReport->setModel(m_model);
  m_tabularReport->setFilter(m_segmentations);
  m_tabularReport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_tabularReport->setMinimumHeight(0);

  QHBoxLayout *informationLayout = new QHBoxLayout();
  informationLayout->addWidget(m_tabularReport);

  m_viewport         ->setLayout(viewportLayout);
  m_historyScrollArea->setWidget(new EmptyHistory());
  m_information      ->setLayout(informationLayout);

  QByteArray geometry = settings.value(SegmentationInspectorSettingsKey, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
    restoreGeometry(geometry);

  QByteArray state = settings.value(SegmentationInspectorSplitterKey, QByteArray()).toByteArray();
  if (!state.isEmpty())
    m_splitter->restoreState(state);

  SegmentationExtension::InfoTagList tags;
  tags << tr("Name") << tr("Category");

  m_viewManager->registerView(m_view);

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(updateSelection()));

  generateWindowTitle();

  updateSelection();
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  ESPINA_SETTINGS(settings);
  settings.setValue(SegmentationInspectorSettingsKey, this->saveGeometry());
  settings.setValue(SegmentationInspectorSplitterKey, m_splitter->saveState());
  settings.sync();

  emit inspectorClosed(this);

  QWidget::closeEvent(e);
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
	m_viewManager->unregisterView(m_view);
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
void SegmentationInspector::addSegmentation(SegmentationAdapterPtr segmentation)
{
  if (m_segmentations.contains(segmentation))
    return;

  m_segmentations << segmentation;
  m_view->add(segmentation);

  auto channels = QueryAdapter::channels(segmentation);

  if(channels.isEmpty())
  {
    qWarning() << "FIXME: Channels shouldn't be empty" << __FILE__ << __LINE__;
    return;
  }

  if (channels.size() > 1)
  {
    qWarning() << "Channel tiling is not supported.";
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
    qWarning() << "Channel tiling is not supported.";
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
  m_tabularReport->updateSelection(m_viewManager->selection()->segmentations());

  m_view->updateView();
  m_view->resetCamera();

  event->acceptProposedAction();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateSelection()
{
  auto activeHistory = m_historyScrollArea->widget();

  if (activeHistory)
  {
    delete activeHistory;
  }

  auto selection = m_viewManager->selection()->segmentations();

  if (selection.size() == 1)
  {
    auto segmentation = selection.first();
    auto delegate     = segmentation->filter()->filterDelegate();
    auto history      = std::dynamic_pointer_cast<FilterHistory>(delegate);

    if (history)
    {
      activeHistory = history->createWidget(m_viewManager, m_undoStack);
    }
    else
    {
      activeHistory = new DefaultHistory(segmentation);
    }
  }
  else
  {
    activeHistory = new EmptyHistory();
  }

  m_historyScrollArea->setWidget(activeHistory);
}
