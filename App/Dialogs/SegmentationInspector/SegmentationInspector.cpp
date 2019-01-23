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
#include <Support/Widgets/TabularReport.h>
#include <GUI/View/View3D.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/View/ViewState.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Managers/ConnectionsManager.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Support/Settings/Settings.h>

// Qt
#include <QSettings>
#include <QLabel>
#include <QLayout>
#include <QDragEnterEvent>
#include <QStack>
#include <QHBoxLayout>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::Support::Representations::Utils;

const QString GEOMETRY_SETTINGS_KEY             = QString("Segmentation Inspector Window Geometry");
const QString INFORMATION_SPLITTER_SETTINGS_KEY = QString("Segmentation Inspector Splitter State");

//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(SegmentationAdapterList        segmentations,
                                             Support::Context              &context)
: QDialog               {DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, WithContext           (context)
, m_selectedSegmentation{nullptr}
, m_channelSources      (getViewState())
, m_segmentationSources (getViewState())
, m_toolbar             {new QToolBar(this)}
, m_view                {context.viewState(), true, this}
, m_tabularReport       (context)
{
  setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);

  connectSignals();

  std::for_each(segmentations.constBegin(), segmentations.constEnd(), [this](const SegmentationAdapterPtr segmentation) { addSegmentation(segmentation); });

  initView3D(context.availableRepresentations());

  initReport();

  configureLayout();

  updateWindowTitle();

  restoreGeometryState();
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  saveGeometryState();

  emit inspectorClosed(this);

  QWidget::closeEvent(e);
}

//------------------------------------------------------------------------
void SegmentationInspector::addSegmentation(SegmentationAdapterPtr segmentation)
{
  if (!m_segmentations.contains(segmentation))
  {
    m_segmentations << segmentation;
    
    for(auto connection: getModel()->connections(getModel()->smartPointer(segmentation)))
    {
      emit connectionAdded(connection);
    }

    auto frame = getViewState().createFrame();
    m_segmentationSources.addSource(toViewItemList(segmentation), frame);

    auto channels = QueryAdapter::channels(segmentation);

    if(channels.isEmpty())
    {
      qWarning() << "FIXME: Channels shouldn't be empty for segmentation" << segmentation->data().toString() << __FILE__ << __LINE__;
    }
    else
    {
      if (channels.size() > 1)
      {
        qWarning() << "Channel tiling is not supported." << __FILE__ << __LINE__;
      }

      addChannel(channels.first().get());
    }
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::removeSegmentation(SegmentationAdapterPtr segmentation)
{
  if (!m_segmentations.contains(segmentation)) return;

  for(auto connection: getModel()->connections(getModel()->smartPointer(segmentation)))
  {
    emit connectionRemoved(connection);
  }

  auto frame = getViewState().createFrame();
  m_segmentationSources.removeSource(toViewItemList(segmentation), frame);

  m_segmentations.removeOne(segmentation);

  if (m_segmentations.isEmpty()) return;

  auto channels = QueryAdapter::channels(segmentation);

  if (channels.size() > 1)
  {
    qWarning() << "Channel tiling is not supported.";
  }
  else
  {
    if(!channels.isEmpty())
    {
      // if there aren't any other segmentations that uses
      // this channel. we must remove it
      auto stackToBeChecked = channels.first();

      bool stillUsed = false;
      int i = 0;

      while(i < m_segmentations.size() && !stillUsed)
      {
        auto remainingSegmentation = m_segmentations.at(i);
        auto remainingStacks = QueryAdapter::channels(remainingSegmentation);
        auto equalOp = [stackToBeChecked](const ChannelAdapterSPtr stack) { return (stack.get() == stackToBeChecked.get()); };

        stillUsed = std::any_of(remainingStacks.constBegin(), remainingStacks.constEnd(), equalOp);
      }

      if (!stillUsed)
      {
        removeChannel(stackToBeChecked.get());
      }
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

    auto frame = getViewState().createFrame();
    m_channelSources.addSource(toViewItemList(channel), frame);

    m_view.refresh();

    updateWindowTitle();
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::removeChannel(ChannelAdapterPtr channel)
{
  if (!m_channels.contains(channel)) return;

  m_channels.removeOne(channel);

  m_view.refresh();

  updateWindowTitle();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateWindowTitle()
{
  auto title = tr("Segmentation Inspector - ");

  if(m_segmentations.isEmpty())
  {
    title += tr("No segmentations");
  }
  else
  {
    for(auto segmentation : m_segmentations)
    {
      title += segmentation->data().toString();
      if(segmentation != m_segmentations.last())
      {
        title += tr(" + ");
      }
    }
  }

  title += tr(" [");

  if(m_channels.empty())
  {
    title += tr("No channel");
  }
  else
  {
    for(auto channel : m_channels)
    {
      title += channel->data().toString();
      if (channel != m_channels.last())
      {
        title += tr(" + ");
      }
    }
  }
  title += tr("]");

  setWindowTitle(title);
}

//------------------------------------------------------------------------
void SegmentationInspector::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  emitConnectionSignals();

  m_tabularReport.updateSelection(getSelectedSegmentations());
}

//------------------------------------------------------------------------
void SegmentationInspector::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
  {
    event->acceptProposedAction();
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::dragMoveEvent(QDragMoveEvent *event)
{
  // TODO: show the drop area of the drag before entering the dialog
  // (probably needs a signal from origin widget to all possible target widgets)
  if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

typedef QMap<int, QVariant> ItemData;

//------------------------------------------------------------------------
void SegmentationInspector::dropEvent(QDropEvent *event)
{
  // TODO + WARNING: this dropEvent() only works for Category (or "Type") layout
  // but right now that is the only layout with drag & drop enabled
  QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  QList<ItemData>         draggedItems;
  SegmentationAdapterList categorySegmentations;
  const auto segmentationsNum = m_segmentations.size();

  while (!stream.atEnd())
  {
    auto model = getModel();

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
  m_tabularReport.updateSelection(getSelectedSegmentations());

  m_view.resetCamera();
  m_view.refresh();

  if(m_segmentations.size() != segmentationsNum)
  {
    updateWindowTitle();

    emit segmentationsUpdated();
  }

  event->acceptProposedAction();
}

//------------------------------------------------------------------------
void SegmentationInspector::configureLayout()
{
  layout()->setMenuBar(m_toolbar);

  m_viewport   ->setLayout(createViewLayout());
  m_information->setLayout(createReportLayout());
}

//------------------------------------------------------------------------
QHBoxLayout *SegmentationInspector::createViewLayout()
{
  auto layout = new QHBoxLayout();

  layout->addWidget(&m_view);
  layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

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

  RepresentationSwitchSList switches;

  for (auto factory : representations)
  {
    auto representation = factory->createRepresentation(getContext(), ViewType::VIEW_3D);

    m_representations << representation;

    switches << representation.Switches;

    for (auto manager : representation.Managers)
    {
      m_view.addRepresentationManager(manager);

      if(manager->name() == "DisplayConnections")
      {
        auto conManager = std::dynamic_pointer_cast<ConnectionsManager>(manager);
        if(conManager) conManager->setConnectionsObject(this);
      }

      for(auto pool: manager->pools())
      {
        if (isStackRepresentation(representation))
        {
          pool->setPipelineSources(&m_channelSources);
        }
        else
        {
          if (isSegmentationRepresentation(representation))
          {
            pool->setPipelineSources(&m_segmentationSources);
          }
        }
      }
    }
  }

  auto comparisonOp = [] (const RepresentationSwitchSPtr &left, const RepresentationSwitchSPtr &right) { if(left == nullptr || right == nullptr) return false; return left->groupWith() < right->groupWith(); };
  std::sort(switches.begin(), switches.end(), comparisonOp);

  for(auto repSwitch: switches)
  {
    for(auto action: repSwitch->actions())
    {
      m_toolbar->addAction(action);
    }
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::initReport()
{
  m_tabularReport.setFilter(m_segmentations);
  m_tabularReport.setModel(getModel());
  m_tabularReport.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_tabularReport.setMinimumHeight(0);
}


//------------------------------------------------------------------------
void SegmentationInspector::restoreGeometryState()
{
  ESPINA_SETTINGS(settings);

  auto geometry = settings.value(GEOMETRY_SETTINGS_KEY, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
  {
    restoreGeometry(geometry);
  }

  auto infoSplitterState = settings.value(INFORMATION_SPLITTER_SETTINGS_KEY, QByteArray()).toByteArray();
  if (!infoSplitterState.isEmpty())
  {
    m_splitter->restoreState(infoSplitterState);
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::saveGeometryState()
{
  ESPINA_SETTINGS(settings);

  settings.setValue(GEOMETRY_SETTINGS_KEY, this->saveGeometry());
  settings.setValue(INFORMATION_SPLITTER_SETTINGS_KEY, m_splitter->saveState());
  settings.sync();
}

//------------------------------------------------------------------------
void SegmentationInspector::connectSignals()
{
  connect(getModel().get(), SIGNAL(segmentationsAboutToBeRemoved(ViewItemAdapterSList)),
          this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

  connect(getModel().get(), SIGNAL(connectionAdded(Connection)),
          this,             SLOT(onConnectionAdded(Connection)));

  connect(getModel().get(), SIGNAL(connectionRemoved(Connection)),
          this,             SLOT(onConnectionRemoved(Connection)));
}

//------------------------------------------------------------------------
void SegmentationInspector::onConnectionAdded(Connection connection)
{
  if(m_segmentations.contains(connection.item1.get()) || m_segmentations.contains(connection.item2.get()))
  {
    emit connectionAdded(connection);
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::onConnectionRemoved(Connection connection)
{
  if(m_segmentations.contains(connection.item1.get()) || m_segmentations.contains(connection.item2.get()))
  {
    emit connectionRemoved(connection);
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::onSegmentationsRemoved(ViewItemAdapterSList segmentations)
{
  const auto segmentationsNum = m_segmentations.size();

  for(auto seg: segmentations)
  {
    auto segPtr = segmentationPtr(seg.get());
    if(m_segmentations.contains(segPtr))
    {
      removeSegmentation(segPtr);

      if(m_segmentations.isEmpty())
      {
        close();
        return;
      }
    }
  }

  if(m_segmentations.size() != segmentationsNum)
  {
    emit segmentationsUpdated();

    updateWindowTitle();

    m_view.refresh();
  }
}

//------------------------------------------------------------------------
void SegmentationInspector::emitConnectionSignals()
{
  for(auto segmentation: m_segmentations)
  {
    auto segSPtr = getModel()->smartPointer(segmentation);
    for(auto connection: getModel()->connections(segSPtr))
    {
      emit connectionAdded(connection);
    }
  }
}
