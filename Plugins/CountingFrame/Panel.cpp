/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Panel.h"
#include "Dialogs/CFTypeSelectorDialog.h"
#include "Extensions/CountingFrameExtension.h"
#include "Extensions/ExtensionUtils.h"

#include "ui_Panel.h"

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>

#include <QFileDialog>
#include <qinputdialog.h>
#include <QPainter>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::CF;

//------------------------------------------------------------------------
class Panel::GUI
: public QWidget
, public Ui::Panel
{
public:
  explicit GUI();

  void setOffsetRanges(int min, int max);
};

//------------------------------------------------------------------------
Panel::GUI::GUI()
{
  setupUi(this);

  leftMargin  ->installEventFilter(this);
  rightMargin ->installEventFilter(this);
  topMargin   ->installEventFilter(this);
  bottomMargin->installEventFilter(this);
  frontMargin ->installEventFilter(this);
  backMargin  ->installEventFilter(this);

  QString tooltip("%1 Face: <br><br> &nbsp; <img src=':/%1.png'> &nbsp; <br>");
  leftMargin  ->setToolTip(tooltip.arg("Left"));
  rightMargin ->setToolTip(tooltip.arg("Right"));
  topMargin   ->setToolTip(tooltip.arg("Top"));
  bottomMargin->setToolTip(tooltip.arg("Bottom"));
  frontMargin ->setToolTip(tooltip.arg("Front"));
  backMargin  ->setToolTip(tooltip.arg("Back"));

  countingFrames->setSortingEnabled(true);
}

//------------------------------------------------------------------------
void Panel::GUI::setOffsetRanges(int min, int max)
{
  leftMargin ->setMinimum(min);
  leftMargin ->setMaximum(max);
  topMargin  ->setMinimum(min);
  topMargin  ->setMaximum(max);
  frontMargin->setMinimum(min);
  frontMargin->setMaximum(max);

  rightMargin ->setMinimum(min);
  rightMargin ->setMaximum(max);
  bottomMargin->setMinimum(min);
  bottomMargin->setMaximum(max);
  backMargin  ->setMinimum(min);
  backMargin  ->setMaximum(max);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
class Panel::CFModel
: public QAbstractTableModel
{
public:
  CFModel(CountingFrameManager *manager)
  : m_manager(manager)
  {}

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const
  { return m_manager->countingFrames().size(); }

  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const
  { return 3; }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  CountingFrame *countingFrame(const QModelIndex &index) const
  { return m_manager->countingFrames()[index.row()]; }

private:
  bool changeId(CountingFrame *editedCF, QString requestedId)
  {
    bool alreadyUsed = false;
    bool accepted    = true;

    for (auto cf : m_manager->countingFrames())
    {
      if (cf != editedCF)
        alreadyUsed |= cf->id() == requestedId;
    }

    if (alreadyUsed)
    {
      QString suggestedId = m_manager->suggestedId(requestedId);
      while (accepted && suggestedId != requestedId)
      {
        requestedId = QInputDialog::getText(nullptr,
                                            tr("Id already used"),
                                            tr("Introduce new id (or accept suggested one)"),
                                            QLineEdit::Normal,
                                            suggestedId,
                                            &accepted);
        suggestedId = m_manager->suggestedId(requestedId);
      }
    }

    if (accepted)
      editedCF->setId(requestedId);

    return accepted;
  }

private:
  CountingFrameManager *m_manager;
};

//------------------------------------------------------------------------
QVariant Panel::CFModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::DisplayRole == role && Qt::Horizontal == orientation)
  {
    switch (section)
    {
      case 0:
        return tr("Id"); break;
      case 1:
        return tr("Type"); break;
      case 2:
        return tr("Channel");
    }
  }

  return QAbstractItemModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant Panel::CFModel::data(const QModelIndex& index, int role) const
{
  auto cf = countingFrame(index);
  int  c  = index.column();

  if (0 == c)
  {
    if (Qt::DisplayRole == role || Qt::EditRole == role)
    {
      return cf->id();
    } else if (Qt::CheckStateRole == role)
    {
      return cf->isVisible()?Qt::Checked:Qt::Unchecked;
    }
  } else if (1 == c && Qt::DisplayRole == role)
  {
      return cf->typeName();
  } else if (2 == c && Qt::DisplayRole == role)
  {
      return cf->extension()->extendedItem()->name();
  }

  return QVariant();
}

//------------------------------------------------------------------------
bool Panel::CFModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (Qt::EditRole == role)
  {
    auto cf = countingFrame(index);

    return changeId(cf, value.toString().trimmed());

  } else if (Qt::CheckStateRole == role)
  {
    auto cf = countingFrame(index);

    cf->setVisible(value.toBool());

    return true;
  }

  return QAbstractItemModel::setData(index, value, role);
}

//------------------------------------------------------------------------
Qt::ItemFlags Panel::CFModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags f = QAbstractItemModel::flags(index);

  if (0 == index.column())
  {
    f = f | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled  | Qt::ItemIsUserCheckable;
  }

  return f;
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
const QString Panel::ID = "CountingFrameExtension";

//------------------------------------------------------------------------
Panel::Panel(CountingFrameManager *manager,
             Support::Context &context)
: m_manager(manager)
, m_context(context)
, m_gui(new GUI())
, m_cfModel(new CFModel(m_manager))
, m_useSlices(true)
, m_activeCF(nullptr)
{
  setObjectName("CountingFrameDock");

  setWindowTitle(tr("Counting Frame"));

  setWidget(m_gui);

  m_gui->countingFrames->setModel(m_cfModel);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

  m_gui->exportCF->setIcon(iconSave);

  connect(m_gui->exportCF, SIGNAL(clicked(bool)),
          this, SLOT(saveActiveCountingFrameDescription()));

  connect(m_gui->createCF, SIGNAL(clicked()),
          this, SLOT(createCountingFrame()));
  connect(m_gui->resetCF, SIGNAL(clicked(bool)),
          this, SLOT(resetActiveCountingFrame()));
  connect(m_gui->deleteCF, SIGNAL(clicked()),
          this, SLOT(deleteActiveCountingFrame()));

  connect(m_gui->countingFrames, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateUI(QModelIndex)));

  connect(m_gui->leftMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->topMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->frontMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->rightMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->bottomMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->backMargin, SIGNAL(valueChanged(double)),
          this, SLOT(updateActiveCountingFrameMargins()));

  connect(m_gui->useCategoryConstraint, SIGNAL(toggled(bool)),
          this, SLOT(enableCategoryConstraints(bool)));
  connect(m_gui->categorySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(applyCategoryConstraint()));

  connect(m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(onCountingFrameCreated(CountingFrame*)));

  connect(m_context.model().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
          this, SLOT(onSegmentationsAdded(ViewItemAdapterSList)));

// TODO   connect(m_viewManager.get(), SIGNAL(activeChannelChanged(ChannelAdapterPtr)),
//           this, SLOT(onChannelChanged(ChannelAdapterPtr)));
}

//------------------------------------------------------------------------
Panel::~Panel()
{
  delete m_cfModel;
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Counting Frame Panel Plugin";
//   qDebug() << "********************************************************";
}

// //------------------------------------------------------------------------
// void Panel::initDockWidget(EspinaModel *model,
//                                         QUndoStack  *undoStack,
//                                         ViewManager *viewManager)
// {
//   m_espinaModel = model;
//   m_viewManager = viewManager;
//
//   Channel::ExtensionPtr channelExtension = new CountingFrameExtension(this, m_viewManager);
//   m_espinaModel->factory()->registerChannelExtension(channelExtension);
//   Segmentation::InformationExtension segExtension = new StereologicalInclusion();
//   m_espinaModel->factory()->registerSegmentationExtension(segExtension);
//   m_espinaModel->factory()->registerRenderer(new CountingFrameRenderer(this));
//
//
//   m_gui->taxonomySelector->setModel(m_espinaModel);
//   m_gui->taxonomySelector->setRootModelIndex(m_espinaModel->taxonomyRoot());
//
//   connect(m_viewManager->fitToSlices(), SIGNAL(toggled(bool)),
//           this, SLOT(changeUnitMode(bool)));
//   changeUnitMode(m_viewManager->fitToSlices()->isChecked());
// }

//------------------------------------------------------------------------
void Panel::reset()
{
  m_gui->countingFrameDescription->clear();

  m_gui->countingFrames->setModel(nullptr);

  m_gui->createCF->setEnabled(false);
  m_gui->deleteCF->setEnabled(false);
  m_gui->resetCF ->setEnabled(false);
  m_gui->exportCF->setEnabled(false);

  m_countingFrames.clear();
}

// //------------------------------------------------------------------------
// IColorEngineProvider::EngineList Panel::colorEngines()
// {
//   EngineList engines;
//   engines << Engine(tr("Counting Frame"), ColorEnginePtr(new CountingFrameColorEngine()));
//
//   return engines;
// }


//------------------------------------------------------------------------
void Panel::deleteCountingFrame(CountingFrame *cf)
{
  if (cf == m_activeCF)
  {
    m_activeCF = nullptr;
  }

  int i = 0;

  while (i < m_pendingCFs.size())
  {
    auto pendingCF = m_pendingCFs[i];

    if (pendingCF.CF == cf)
    {
      pendingCF.Task->abort();

      m_pendingCFs.at(i);

      i = m_pendingCFs.size();
    }
  }

  m_countingFrames.removeOne(cf);

  disconnect(cf,   SIGNAL(modified(CountingFrame*)),
             this, SLOT(showInfo(CountingFrame*)));
  disconnect(cf,   SIGNAL(applied(CountingFrame*)),
             this, SLOT(onCountingFrameApplied(CountingFrame*)));

  cf->deleteFromExtension();

  updateTable();

  updateUI(m_gui->countingFrames->model()->index(0,0));
}

//------------------------------------------------------------------------
void Panel::applyCategoryConstraint()
{
  if (m_activeCF && m_gui->useCategoryConstraint->isChecked())
  {
    QModelIndex categoryyIndex = m_gui->categorySelector->currentModelIndex();
    if (categoryyIndex.isValid())
    {
      auto item = itemAdapter(categoryyIndex);
      Q_ASSERT(isCategory(item));

      auto category = categoryPtr(item);

      m_activeCF->setCategoryConstraint(category->classificationName());
    }
  }
}

//------------------------------------------------------------------------
void Panel::enableCategoryConstraints(bool enable)
{
  m_gui->categorySelector->setEnabled(enable);

  applyCategoryConstraint();
}

//------------------------------------------------------------------------
void Panel::updateUI(QModelIndex index)
{
  bool validCF = !m_countingFrames.isEmpty() && index.isValid();

  if (validCF)
  {
    CountingFrame *cf;// = m_countingFrames.value(index.row(), nullptr);
    cf = m_cfModel->countingFrame(index);
    Q_ASSERT(cf);

    showInfo(cf);
  } else
  {
    m_activeCF = nullptr;

    m_gui->leftMargin  ->setValue(0);
    m_gui->topMargin   ->setValue(0);
    m_gui->frontMargin ->setValue(0);
    m_gui->rightMargin ->setValue(0);
    m_gui->bottomMargin->setValue(0);
    m_gui->backMargin  ->setValue(0);

    m_gui->useCategoryConstraint->setChecked(validCF);

    m_gui->countingFrameDescription->clear();

    m_gui->countingFrames->setFocus();
  }

  m_gui->countingFrames->setEnabled(validCF);

  m_gui->leftMargin  ->setEnabled(validCF);
  m_gui->topMargin   ->setEnabled(validCF);
  m_gui->frontMargin ->setEnabled(validCF);
  m_gui->rightMargin ->setEnabled(validCF);
  m_gui->bottomMargin->setEnabled(validCF);
  m_gui->backMargin  ->setEnabled(validCF);

  m_gui->countingFrameDescription->setEnabled(validCF);

  m_gui->useCategoryConstraint->setEnabled(validCF);
  m_gui->categorySelector     ->setEnabled(m_gui->useCategoryConstraint->isChecked());

  m_gui->deleteCF->setEnabled(validCF);
  m_gui->resetCF ->setEnabled(validCF);
  m_gui->exportCF->setEnabled(validCF);
}

//------------------------------------------------------------------------
void Panel::createCountingFrame()
{
  if (!m_pendingCFs.isEmpty()) return;

  CFTypeSelectorDialog cfSelector(m_context.model(), this);

  if (cfSelector.exec())
  {
    CFType type        = cfSelector.type();
    QString constraint = cfSelector.categoryConstraint();

    auto channel = cfSelector.channel();
    Q_ASSERT(channel);

    if (!channel->hasExtension(CountingFrameExtension::TYPE))
    {
      channel->addExtension(m_manager->createExtension(m_context.scheduler()));
    }


    NmVector3 spacing = channel->output()->spacing();

    Nm inclusion[3];
    Nm exclusion[3];
    for(int i = 0; i < 3; ++i)
    {
      inclusion[i] = exclusion[i] = spacing[i]/2;
    }

    auto extension = retrieveExtension<CountingFrameExtension>(channel);

    extension->createCountingFrame(type, inclusion, exclusion, constraint);

    //m_gui->createCF->setEnabled(false);
  }
//
//   updateSegmentations();
}

//------------------------------------------------------------------------
void Panel::resetActiveCountingFrame()
{
  if (m_activeCF)
  {
    auto channel       = m_activeCF->channel();
    auto segmentations = QueryContents::segmentationsOnChannelSample(channel);

    ComputeOptimalMarginsSPtr task(new ComputeOptimalMarginsTask(channel, segmentations, m_context.scheduler()));

    connect(task.get(), SIGNAL(finished()),
            this,       SLOT(onMarginsComputed()));
//     connect(task.get(), SIGNAL(progress(int)),
//             this,       SLOT(reportProgess(int)));

    m_pendingCFs << PendingCF(m_activeCF, task);

    Task::submit(task);
  }
}

//------------------------------------------------------------------------
void Panel::updateActiveCountingFrameMargins()
{
  if (!m_activeCF)
    return;

  Nm inclusion[3];
  Nm exclusion[3];

  inclusionMargins(inclusion);
  exclusionMargins(exclusion);

  m_activeCF->setMargins(inclusion, exclusion);
}


//------------------------------------------------------------------------
void Panel::deleteActiveCountingFrame()
{
  if (!m_activeCF)
    return;

  deleteCountingFrame(m_activeCF);

  updateSegmentations();
}

//------------------------------------------------------------------------
void Panel::onChannelChanged(ChannelAdapterPtr channel)
{
  auto model = m_context.model().get();

  m_gui->categorySelector->setModel(model);
  m_gui->categorySelector->setRootModelIndex(model->classificationRoot());

  m_gui->createCF->setEnabled(channel != nullptr);

  if (channel)
  {
    Bounds bounds = channel->output()->bounds();

    double lenght[3];
    for (int i=0; i < 3; i++)
      lenght[i] = bounds[2*i+1]-bounds[2*i];

    m_gui->leftMargin  ->setMaximum(lenght[0]);
    m_gui->topMargin   ->setMaximum(lenght[1]);
    m_gui->frontMargin ->setMaximum(lenght[2]);
    m_gui->rightMargin ->setMaximum(lenght[0]);
    m_gui->bottomMargin->setMaximum(lenght[1]);
    m_gui->backMargin  ->setMaximum(lenght[2]);
  }
  else
  {
    m_gui->setOffsetRanges(0,0);
  }
}

//------------------------------------------------------------------------
void Panel::showInfo(CountingFrame* activeCF)
{
  if (!activeCF || !m_context.ActiveChannel)
    return;

  m_activeCF = activeCF;

  int row = m_countingFrames.indexOf(activeCF);

  auto selectionModel = m_gui->countingFrames->selectionModel();
  auto index = m_cfModel->index(row, 0);
  selectionModel->select(index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);

  m_gui->leftMargin  ->blockSignals(true);
  m_gui->topMargin   ->blockSignals(true);
  m_gui->frontMargin ->blockSignals(true);
  m_gui->rightMargin ->blockSignals(true);
  m_gui->bottomMargin->blockSignals(true);
  m_gui->backMargin  ->blockSignals(true);

  auto channel = activeCF->extension()->extendedItem();
  auto spacing = channel->output()->spacing();

  m_gui->leftMargin  ->setSingleStep(spacing[0]);
  m_gui->rightMargin ->setSingleStep(spacing[0]);
  m_gui->topMargin   ->setSingleStep(spacing[1]);
  m_gui->bottomMargin->setSingleStep(spacing[1]);

  m_gui->leftMargin  ->setValue(activeCF->left());
  m_gui->topMargin   ->setValue(activeCF->top() );
  m_gui->rightMargin ->setValue(activeCF->right() );
  m_gui->bottomMargin->setValue(activeCF->bottom());
  if (m_useSlices)
  {
    m_gui->frontMargin->setSingleStep(1);
    m_gui->backMargin ->setSingleStep(1);

    m_gui->frontMargin ->setValue(int(activeCF->front()/spacing[2]));
    m_gui->backMargin  ->setValue(int(activeCF->back()/spacing[2]));
  } else
  {
    m_gui->frontMargin->setSingleStep(spacing[2]);
    m_gui->backMargin ->setSingleStep(spacing[2]);

    m_gui->frontMargin ->setValue(activeCF->front());
    m_gui->backMargin  ->setValue(activeCF->back());
  }


  m_gui->leftMargin  ->blockSignals(false);
  m_gui->topMargin   ->blockSignals(false);
  m_gui->frontMargin ->blockSignals(false);
  m_gui->rightMargin ->blockSignals(false);
  m_gui->bottomMargin->blockSignals(false);
  m_gui->backMargin  ->blockSignals(false);

  auto applyCaregoryConstraint = !activeCF->categoryConstraint().isEmpty();
  if (applyCaregoryConstraint)
  {
    m_gui->categorySelector->setCurrentModelIndex(findCategoryIndex(activeCF->categoryConstraint()));
  }
  m_gui->useCategoryConstraint->blockSignals(true);
  m_gui->useCategoryConstraint->setChecked(applyCaregoryConstraint);
  m_gui->useCategoryConstraint->blockSignals(false);

  m_gui->countingFrameDescription->setText(activeCF->description());

  for (auto cf : m_countingFrames)
  {
    cf->setHighlighted(cf == activeCF);
  }
}

//------------------------------------------------------------------------
QModelIndex Panel::findCategoryIndex(const QString& classificationName)
{
  auto model    = m_context.model();
  auto category = model->classification()->category(classificationName);

  return model->categoryIndex(category);
}

//------------------------------------------------------------------------
void Panel::updateSegmentations()
{
  auto model         = m_context.model();
  auto segmentations = toRawList<ViewItemAdapter>(model->segmentations());

  m_context.representationInvalidator().invalidateRepresentations(segmentations);
}


//------------------------------------------------------------------------
void Panel::saveActiveCountingFrameDescription()
{
  auto title    = tr("Save Counting Frame Description");
  auto formats  = SupportedFiles(tr("Text File"), "txt");
//                       .addFormat(tr("Excel Sheet"), "xls");
  auto fileName = DefaultDialogs::SaveFile(title, formats);

  if (!fileName.isEmpty())
  {
    if (fileName.endsWith(".txt"))
    {
      exportCountingFrameDescriptionAsText(fileName);
    }
    else if (fileName.endsWith(".xls"))
    {
      exportCountingFrameDescriptionAsExcel(fileName);
    }
  }
}

//------------------------------------------------------------------------
void Panel::changeUnitMode(bool useSlices)
{
  m_useSlices = useSlices;

  if (useSlices)
  {
    m_gui->frontMargin->setSuffix("");
    m_gui->backMargin ->setSuffix("");
  } else
  {
    m_gui->frontMargin->setSuffix(" nm");
    m_gui->backMargin ->setSuffix(" nm");
  }

  showInfo(m_activeCF);
}

//------------------------------------------------------------------------
void Panel::reportProgess(int progress)
{
  QIcon icon(":/create-cf.svg");

  auto size = m_gui->createCF->size();
  QPixmap original = icon.pixmap(size);
  QPixmap inverted = icon.pixmap(size, QIcon::Disabled);

//   QPainter painter(&pixmap);
//   QRect rect = pixmap.rect();
//   QLinearGradient gradient(rect.bottomLeft() - QPoint(0,progress), rect.topLeft());
//   gradient.setColorAt(0, QColor(0,0,0,255));
//   gradient.setColorAt(1, QColor(0,0,0,50));
//   painter.fillRect(rect, gradient);
//   painter.fillRect(rect, QColor(125,125,125,125));

  QImage originalImage = original.toImage();
  QImage invertedImage = inverted.toImage();

  int width  = original.width();
  int height = original.height();

  for (int i = 0; i < width; ++i)
  {
    for (int j = 0; j < (100-progress)*height/100; ++j)
    {
      originalImage.setPixel(i, j, invertedImage.pixel(i, j));
    }
  }
  original = original.fromImage(originalImage);

  m_gui->createCF->setIcon(original);
}

//------------------------------------------------------------------------
// WARNING: if further changes are needed unify implementation
void Panel::computeOptimalMargins(ChannelAdapterPtr channel,
                                  Nm inclusion[3],
                                  Nm exclusion[3])
{
  auto spacing = channel->output()->spacing();

  memset(inclusion, 0, 3*sizeof(Nm));
  memset(exclusion, 0, 3*sizeof(Nm));

  const NmVector3 delta{ 0.5*spacing[0], 0.5*spacing[1], 0.5*spacing[2] };

  QApplication::setOverrideCursor(Qt::WaitCursor);
  for (auto segmentation : QueryAdapter::segmentationsOnChannelSample(channel))
  {
    auto extension = retrieveOrCreateExtension<EdgeDistance>(segmentation);

    Nm dist2Margin[6];
    extension->edgeDistance(dist2Margin);

    auto bounds  = segmentation->output()->bounds();
    auto spacing = segmentation->output()->spacing();

    for (int i=0; i < 3; i++)
    {
      Nm shift  = i < 2? 0.5:-0.5;
      Nm length = bounds.lenght(toAxis(i));

      if (dist2Margin[2*i] < delta[i])
        inclusion[i] = (vtkMath::Round(std::max(length, inclusion[i])/spacing[i]-shift)+shift)*spacing[i];
      //         if (dist2Margin[2*i+1] < delta[i])
      //           exclusion[i] = std::max(length, exclusion[i]);
    }
  }
  QApplication::restoreOverrideCursor();
//   qDebug() << "Inclusion:" << inclusion[0] << inclusion[1] << inclusion[2];
//   qDebug() << "Exclusion:" << exclusion[0] << exclusion[1] << exclusion[2];
}

// //------------------------------------------------------------------------
// // WARNING: if further changes are needed unify implementation
// void Panel::computeOptimalMargins(ChannelPtr channel,
//                                   Nm inclusion[3],
//                                   Nm exclusion[3])
// {
//   auto spacing = channel->output()->spacing();
//
//   memset(inclusion, 0, 3*sizeof(Nm));
//   memset(exclusion, 0, 3*sizeof(Nm));
//
//   const NmVector3 delta{ 0.5*spacing[0], 0.5*spacing[1], 0.5*spacing[2] };
//
//   QApplication::setOverrideCursor(Qt::WaitCursor);
//   for (auto segmentation : Query::segmentationsOnChannelSample(channel))
//   {
//     auto extension = retrieveOrCreateExtension<EdgeDistance>(segmentation);
//
//     Nm dist2Margin[6];
//     extension->edgeDistance(dist2Margin);
//
//     auto bounds  = segmentation->output()->bounds();
//     auto spacing = segmentation->output()->spacing();
//
//     for (int i=0; i < 3; i++)
//     {
//       Nm shift  = i < 2? 0.5:-0.5;
//       Nm length = bounds.lenght(toAxis(i));
//
//       if (dist2Margin[2*i] < delta[i])
//         inclusion[i] = (vtkMath::Round(std::max(length, inclusion[i])/spacing[i]-shift)+shift)*spacing[i];
//       //         if (dist2Margin[2*i+1] < delta[i])
//       //           exclusion[i] = std::max(length, exclusion[i]);
//     }
//   }
//   QApplication::restoreOverrideCursor();
// //   qDebug() << "Inclusion:" << inclusion[0] << inclusion[1] << inclusion[2];
// //   qDebug() << "Exclusion:" << exclusion[0] << exclusion[1] << exclusion[2];
// }


//------------------------------------------------------------------------
void Panel::inclusionMargins(double values[3])
{
  values[0] = m_gui->leftMargin ->value();
  values[1] = m_gui->topMargin  ->value();
  values[2] = m_gui->frontMargin->value();

  if (m_useSlices)
  {
    auto channel = m_activeCF->extension()->extendedItem();
    auto spacing = channel->output()->spacing();

    values[2] *= spacing[2];
  }
}

//------------------------------------------------------------------------
void Panel::exclusionMargins(double values[3])
{
  values[0] = m_gui->rightMargin ->value();
  values[1] = m_gui->bottomMargin->value();
  values[2] = m_gui->backMargin  ->value();

  if (m_useSlices)
  {
    auto channel = m_activeCF->extension()->extendedItem();
    auto spacing = channel->output()->spacing();

    values[2] = (m_gui->backMargin->value() + 0.5) * spacing[2];
  }
}

//------------------------------------------------------------------------
void Panel::onMarginsComputed()
{
  TaskPtr task = dynamic_cast<TaskPtr>(sender());

  PendingCF                 pendingCF;
  ComputeOptimalMarginsSPtr optimalMargins;

  for(auto cf : m_pendingCFs)
  {
    TaskPtr basePtr = cf.Task.get();
    if (basePtr == task)
    {
      pendingCF      = cf;
      optimalMargins = cf.Task;
      break;
    }
  }

  Q_ASSERT(pendingCF.Task);

  m_pendingCFs.removeOne(pendingCF);

  Nm inclusion[3] = {0, 0, 0};
  Nm exclusion[3] = {0, 0 ,0};

  //auto channel = optimalMargins->channel();
  optimalMargins->inclusion(inclusion);

  pendingCF.CF->setMargins(inclusion, exclusion);
  //m_gui->createCF->setEnabled(m_pendingCFs.isEmpty());
}


//------------------------------------------------------------------------
void Panel::onCountingFrameCreated(CountingFrame* cf)
{
  connect(cf,   SIGNAL(modified(CountingFrame*)),
          this, SLOT(showInfo(CountingFrame*)));
  connect(cf,   SIGNAL(applied(CountingFrame*)),
          this, SLOT(onCountingFrameApplied(CountingFrame*)));

  cf->apply();

  m_countingFrames << cf;

  updateTable();

  //m_viewManager->addWidget(cf);

  m_activeCF = cf; // To make applyCategoryConstraint work

  updateUI(m_cfModel->index(m_cfModel->rowCount() - 1, 0));
}

//------------------------------------------------------------------------
void Panel::onCountingFrameApplied(CountingFrame *cf)
{
  updateSegmentations();
}

//------------------------------------------------------------------------
void Panel::onSegmentationsAdded(ViewItemAdapterSList items)
{
  SegmentationAdapterSList segmentations;
  for (auto item : items)
  {
    segmentations << std::dynamic_pointer_cast<SegmentationAdapter>(item);
  }

  if (!m_manager->countingFrames().isEmpty())
  {
    applyCountingFrames(segmentations);
  }
}

//------------------------------------------------------------------------
void Panel::updateTable()
{
  m_gui->countingFrames->setModel(nullptr);
  m_gui->countingFrames->setModel(m_cfModel);
}

//------------------------------------------------------------------------
void Panel::exportCountingFrameDescriptionAsText(const QString &filename)
{
  QFile file(filename);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);

  QTextStream out(&file);
  out << m_gui->countingFrameDescription->toPlainText();

  file.close();
}

//------------------------------------------------------------------------
void Panel::exportCountingFrameDescriptionAsExcel(const QString& filename)
{

}

//------------------------------------------------------------------------
void Panel::applyCountingFrames(SegmentationAdapterSList segmentations)
{
  for (auto segmentation : segmentations)
  {
    auto sterologicalExtension = retrieveOrCreateExtension<StereologicalInclusion>(segmentation);

    auto samples = QueryAdapter::samples(segmentation);
    Q_ASSERT(samples.size() == 1);

    for (auto channel : QueryAdapter::channels(samples[0]))
    {
      if (channel->hasExtension(CountingFrameExtension::TYPE))
      {
        auto cfExtension = retrieveExtension<CountingFrameExtension>(channel);

        for (auto cf : cfExtension->countingFrames())
        {
          sterologicalExtension->addCountingFrame(cf);
          cf->apply();
        }
      }
    }
  }
}

//------------------------------------------------------------------------
void Panel::deleteCountingFrames()
{
  for(auto cf: m_countingFrames)
  {
    deleteCountingFrame(cf);
  }
}
