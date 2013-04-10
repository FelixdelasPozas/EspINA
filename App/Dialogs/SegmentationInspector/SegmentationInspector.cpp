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

// EspINA
#include <Core/Model/ModelItem.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Model/Filter.h>
#include <Core/Model/ModelTest.h>
#include <GUI/QtWidget/VolumeView.h>
#include <Core/EspinaSettings.h>

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
SegmentationInspector::SegmentationInspector(SegmentationList segmentations,
                                             EspinaModel     *model,
                                             QUndoStack      *undoStack,
                                             ViewManager     *viewManager,
                                             QWidget         *parent,
                                             Qt::WindowFlags  flags)
: QWidget(parent, flags|Qt::WindowStaysOnTopHint)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_tabularReport(new TabularReport(viewManager))
, m_view(new VolumeView(model->factory(), viewManager, true))
, m_filterArea(new QScrollArea(this))
{
  m_view->setViewType(VOLUME);

  //ModelTest * m_modelTester = new ModelTest(m_info.data());

  setupUi(this);
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setAcceptDrops(true);

  foreach(SegmentationPtr seg, segmentations)
  {
    addSegmentation(seg);
    connect(seg, SIGNAL(modified(ModelItemPtr)),
            this, SLOT(updateScene(ModelItemPtr)));
  }

  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->resetCamera();
  m_view->updateView();

  Segmentation::InfoTagList tags;
  tags << tr("Name") << tr("Taxonomy");
  foreach(Segmentation::InformationExtension extension, m_model->factory()->segmentationExtensions())
  {
    if (extension->validTaxonomy(""))
      tags << extension->availableInformations();
  }

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

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection,bool)),
          this, SLOT(updateSelection(ViewManager::Selection)));

  QSettings settings(CESVIMA, ESPINA);
  QByteArray geometry = settings.value(SegmentationInspectorSettingsKey, QByteArray()).toByteArray();
  if (!geometry.isEmpty())
    restoreGeometry(geometry);

  QByteArray state = settings.value(SegmentationInspectorSplitterKey, QByteArray()).toByteArray();
  if (!state.isEmpty())
    m_splitter->restoreState(state);

  generateWindowTitle();
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
}

//------------------------------------------------------------------------
void SegmentationInspector::updateScene(ModelItemPtr item)
{
  SegmentationPtr seg = segmentationPtr(item);
  m_view->updateSegmentation(seg);
  m_view->updateView();
}

//------------------------------------------------------------------------
void SegmentationInspector::updateSelection(ViewManager::Selection selection)
{
  QWidget *prevWidget = m_filterArea->widget();
  if (prevWidget)
    delete prevWidget;

  QWidget *noWidgetInspector = new QWidget();
  noWidgetInspector->setLayout(new QVBoxLayout());
  noWidgetInspector->layout()->setSpacing(10);
  noWidgetInspector->setMaximumWidth(250);

  QLabel *infoLabel = new QLabel(noWidgetInspector);

  if (selection.size() == 1 && EspINA::SEGMENTATION == selection.first()->type() && m_segmentations.contains(segmentationPtr(selection.first())))
  {
    Filter::FilterInspectorPtr inspector = segmentationPtr(selection.first())->filter()->filterInspector();
    if (inspector)
    {
      delete noWidgetInspector;
      QWidget *inspectorwidget = inspector->createWidget(m_undoStack, m_viewManager);
      inspectorwidget->setMaximumWidth(250);
      m_filterArea->setWidget(inspectorwidget);
      return;
    }
    else
    {
      infoLabel->setText(QLabel::tr("This filter doesn't have configurable options."));
    }
  }
  else
  {
    infoLabel->setText(QLabel::tr("No segmentation selected."));
  }

  infoLabel->setWordWrap(true);
  infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
  noWidgetInspector->layout()->addWidget(infoLabel);

  QSpacerItem* spacer = new QSpacerItem(-1, -1, QSizePolicy::Minimum, QSizePolicy::Expanding);
  noWidgetInspector->layout()->addItem(spacer);

  m_filterArea->setWidget(noWidgetInspector);
}


//------------------------------------------------------------------------
void SegmentationInspector::addSegmentation(SegmentationPtr segmentation)
{
  if (m_segmentations.contains(segmentation))
    return;

  m_segmentations << segmentation;
  m_view->addSegmentation(segmentation);

  addChannel(segmentation->channel().data());
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
void SegmentationInspector::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  updateSelection(m_viewManager->selection());
  m_tabularReport->updateSelection(m_viewManager->selection());
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
  // TODO + WARNING: this dropEvent() only works for Taxonomy (or "Type") layout
  // but right now that is the only layout with drag & drop enabled

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

  m_tabularReport->setFilter(m_segmentations);

  updateSelection(m_viewManager->selection());
  m_tabularReport->updateSelection(m_viewManager->selection());

  m_view->updateView();
  m_view->resetCamera();

  event->acceptProposedAction();
}
