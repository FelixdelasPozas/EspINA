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
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/View/ViewState.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/FilterHistory.h>
#include <Support/Representations/RepresentationUtils.h>

// Qt
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QDragEnterEvent>
#include <QStack>
#include <QHBoxLayout>
#include <QDebug>

using namespace ESPINA::Support;

namespace ESPINA
{
  const QString SegmentationInspectorSettingsKey = QString("Segmentation Inspector Window Geometry");
  const QString SegmentationInspectorSplitterKey = QString("Segmentation Inspector Splitter State");
}

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Support::Representations::Utils;

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(SegmentationAdapterList   segmentations,
                                             FilterDelegateFactorySPtr delegateFactory,
                                             Support::Context   &context)
: QWidget               {nullptr, Qt::WindowStaysOnTopHint}
, m_context             (context)
, m_delegateFactory     {delegateFactory}
, m_selectedSegmentation{nullptr}
, m_channelSources      {context.representationInvalidator()}
, m_segmentationSources {context.representationInvalidator()}
, m_representations     {context.timer()}
, m_view                {context.viewState(), true}
, m_tabularReport       {context}
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);

  for(auto segmentation : segmentations)
  {
    addSegmentation(segmentation);
  }

  initView3D(m_context.availableRepresentations());

  initReport();

  configureLayout();

  restoreGeometryState();

   connect(selection().get(), SIGNAL(selectionChanged()),
           this,              SLOT(updateSelection()));

  updateWindowTitle();

  updateSelection();
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  saveGeometry();

  emit inspectorClosed(this);

  QWidget::closeEvent(e);
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
}

//------------------------------------------------------------------------
void SegmentationInspector::addSegmentation(SegmentationAdapterPtr segmentation)
{
  if (!m_segmentations.contains(segmentation))
  {
    m_segmentations << segmentation;
    m_segmentationSources.addSource(toViewItemList(segmentation), m_view.timeStamp());

    auto channels = QueryAdapter::channels(segmentation);

    if(channels.isEmpty())
    {
      qWarning() << "FIXME: Channels shouldn't be empty" << __FILE__ << __LINE__;
    }
    else
    {
      if (channels.size() > 1)
      {
        qWarning() << "Channel tiling is not supported.";
      }

      addChannel(channels.first().get());
    }
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

  //TODO use sources and pools m_view->remove(segmentation);

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

  m_view.refresh();

  updateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::addChannel(ChannelAdapterPtr channel)
{
  if (!m_channels.contains(channel))
  {
    m_channels << channel;
    m_channelSources.addSource(toViewItemList(channel), m_view.timeStamp());

    m_channels << channel;

    m_view.refresh();

    updateWindowTitle();
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::removeChannel(ChannelAdapterPtr channel)
{
  if (!m_channels.contains(channel))
    return;

  m_channels.removeOne(channel);

  m_view.refresh();

  updateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateWindowTitle()
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
SelectionSPtr SegmentationInspector::selection() const
{
  return ESPINA::Support::getSelection(m_context);
}


//------------------------------------------------------------------------
void SegmentationInspector::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  m_tabularReport.updateSelection(selection()->segmentations());
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
    auto model = m_context.model();

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
        auto category = toCategoryAdapterPtr(item);

        for(auto segmentation : model->segmentations())
        {
          auto segmentationCategory = segmentation->category().get();
          while(segmentationCategory != model->classification()->root().get())
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

  m_tabularReport.setFilter(m_segmentations);
  m_tabularReport.updateSelection(selection()->segmentations());

  m_view.resetCamera();
  m_view.refresh();

  event->acceptProposedAction();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateSelection()
{
  auto activeHistory = m_historyScrollArea->widget();

  if (activeHistory)
  {
    delete activeHistory;

    activeHistory = nullptr;
  }

  auto selectedSegmentations = selection()->segmentations();

  if (selectedSegmentations.size() == 1)
  {
    auto segmentation = selectedSegmentations.first();

    if (m_segmentations.contains(segmentation))
    {
      if (m_selectedSegmentation != segmentation)
      {
        disconnect(m_selectedSegmentation, SIGNAL(outputModified()),
                   this, SLOT(updateSelection()));

        m_selectedSegmentation = segmentation;

        connect(m_selectedSegmentation, SIGNAL(outputModified()),
                this, SLOT(updateSelection()));
      }

      try
      {
        auto delegate = m_delegateFactory->createDelegate(segmentation);
        activeHistory = delegate->createWidget(m_context);
      }
      catch (...)
      {
        activeHistory = new DefaultHistory(segmentation);
      }
    }
  }

  if (nullptr == activeHistory)
  {
    activeHistory = new EmptyHistory();
  }

  m_historyScrollArea->setWidget(activeHistory);
}

//------------------------------------------------------------------------
void SegmentationInspector::configureLayout()
{
  layout()->setMenuBar(&m_toolbar);
//   m_historyScrollArea->widget()->setMinimumWidth(250);
//   m_historyScrollArea->setMinimumWidth(150);

//   QVBoxLayout *historyLayout = new QVBoxLayout();
//   historyLayout->addWidget(m_historyScrollArea);

  m_viewport         ->setLayout(createViewLayout());
  m_historyScrollArea->setWidget(new EmptyHistory());
  m_information      ->setLayout(createReportLayout());
}

//------------------------------------------------------------------------
QVBoxLayout *SegmentationInspector::createViewLayout()
{
  auto layout = new QVBoxLayout();

  layout->addWidget(&m_view);
  layout->setSizeConstraint(QLayout::SetMinimumSize);

  return layout;
}

//------------------------------------------------------------------------
QHBoxLayout *SegmentationInspector::createReportLayout()
{
  auto layout = new QHBoxLayout();

  layout->addWidget(&m_tabularReport);

  return layout;
}

//------------------------------------------------------------------------
void SegmentationInspector::initView3D(RepresentationFactorySList representations)
{
  m_view.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  m_view.setMinimumWidth(250);

  for (auto factory : representations)
  {
    auto representation = factory->createRepresentation(m_context, ViewType::VIEW_3D);

    for (auto repSwitch : representation.Switches)
    {
      m_representations.addRepresentationSwitch(representation.Group, repSwitch, representation.Icon, representation.Description);
    }

    for (auto manager : representation.Managers)
    {
      m_view.addRepresentationManager(manager);
    }

    for (auto pool : representation.Pools)
    {
      if (isChannelRepresentation(representation))
      {
        pool->setPipelineSources(&m_channelSources);
      }
      else if (isSegmentationRepresentation(representation))
      {
        pool->setPipelineSources(&m_segmentationSources);
      }
    }
  }

  for (auto tool : m_representations.representationTools())
  {
    for (auto action : tool->actions())
    {
      m_toolbar.addAction(action);
    }
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::initReport()
{
  SegmentationExtension::InfoTagList tags;
  tags << tr("Name") << tr("Category");

  m_tabularReport.setModel(m_context.model());
  m_tabularReport.setFilter(m_segmentations);
  m_tabularReport.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_tabularReport.setMinimumHeight(0);
}


//------------------------------------------------------------------------
void SegmentationInspector::restoreGeometryState()
{
  ESPINA_SETTINGS(settings);

  QByteArray geometry = settings.value(SegmentationInspectorSettingsKey, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
  {
    restoreGeometry(geometry);
  }

  QByteArray state = settings.value(SegmentationInspectorSplitterKey, QByteArray()).toByteArray();
  if (!state.isEmpty())
  {
    m_splitter->restoreState(state);
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::saveGeometryState()
{
  ESPINA_SETTINGS(settings);

  settings.setValue(SegmentationInspectorSettingsKey, this->saveGeometry());
  settings.setValue(SegmentationInspectorSplitterKey, m_splitter->saveState());
  settings.sync();
}
