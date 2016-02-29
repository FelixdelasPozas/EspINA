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

// Plugin
#include "Panel.h"
#include "Dialogs/CFTypeSelectorDialog.h"
#include "Extensions/CountingFrameExtension.h"
#include "Extensions/ExtensionUtils.h"
#include "ui_Panel.h"

// ESPINA
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>
#include <Support/Utils/xlsUtils.h>

// Qt
#include <QInputDialog>
#include <QPainter>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::CF;
using namespace xlslib_core;

//------------------------------------------------------------------------
class CF::Panel::GUI
: public QWidget
, public Ui::Panel
{
public:
  explicit GUI();

  void setOffsetRanges(int min, int max);
};

//------------------------------------------------------------------------
CF::Panel::GUI::GUI()
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

  countingFrames->setSortingEnabled(false);
}

//------------------------------------------------------------------------
void CF::Panel::GUI::setOffsetRanges(int min, int max)
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
class CF::Panel::CFModel
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
    { return m_manager->countingFrames().at(index.row()); }

  private:
    bool changeId(CountingFrame *editedCF, QString requestedId)
    {
      bool alreadyUsed = false;
      bool accepted    = true;

      for (auto cf : m_manager->countingFrames())
      {
        if (cf != editedCF)
        {
          alreadyUsed |= cf->id() == requestedId;
        }
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
      {
        editedCF->setId(requestedId);
      }

      return accepted;
    }

  private:
    CountingFrameManager *m_manager;
};

//------------------------------------------------------------------------
QVariant CF::Panel::CFModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        return tr("Stack");
    }
  }

  return QAbstractItemModel::headerData(section, orientation, role);
}

//------------------------------------------------------------------------
QVariant CF::Panel::CFModel::data(const QModelIndex& index, int role) const
{
  auto cf = countingFrame(index);
  int  c  = index.column();

  if (0 == c)
  {
    if (Qt::DisplayRole == role || Qt::EditRole == role)
    {
      return cf->id();
    }
    else
    {
      if (Qt::CheckStateRole == role)
      {
        return cf->isVisible() ? Qt::Checked : Qt::Unchecked;
      }
    }
  }
  else
  {
    if (1 == c && Qt::DisplayRole == role)
    {
      return cf->typeName();
    }
    else
    {
      if (2 == c && Qt::DisplayRole == role)
      {
        return cf->extension()->extendedItem()->name();
      }
    }
  }

  return QVariant();
}

//------------------------------------------------------------------------
bool CF::Panel::CFModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (Qt::EditRole == role)
  {
    auto cf = countingFrame(index);

    return changeId(cf, value.toString().trimmed());
  }
  else
  {
    if (Qt::CheckStateRole == role)
    {
      auto cf = countingFrame(index);

      cf->setVisible(value.toBool());

      return true;
    }
  }

  return QAbstractItemModel::setData(index, value, role);
}

//------------------------------------------------------------------------
Qt::ItemFlags CF::Panel::CFModel::flags(const QModelIndex& index) const
{
  auto flags = QAbstractItemModel::flags(index);

  if (0 == index.column())
  {
    flags = flags | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled  | Qt::ItemIsUserCheckable;
  }

  return flags;
}

//------------------------------------------------------------------------
const QString CF::Panel::ID = "CountingFrameExtension";

//------------------------------------------------------------------------
CF::Panel::Panel(CountingFrameManager *manager, Support::Context &context)
: ESPINA::Panel(tr("Counting Frame Dock"), context)
, m_manager  {manager}
, m_context  (context)
, m_gui      {new GUI()}
, m_cfModel  {new CFModel(m_manager)}
, m_useSlices{true}
, m_activeCF {nullptr}
{
  setObjectName("CountingFrameDock");

  setWindowTitle(tr("Counting Frame"));

  setWidget(m_gui);

  m_gui->countingFrames->setModel(m_cfModel);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);

  m_gui->exportCF->setIcon(iconSave);

  connect(m_gui->exportCF, SIGNAL(clicked(bool)),
          this,            SLOT(exportCountingFramesData()));

  connect(m_gui->createCF, SIGNAL(clicked()),
          this,            SLOT(createCountingFrame()));
  connect(m_gui->resetCF, SIGNAL(clicked(bool)),
          this,           SLOT(resetActiveCountingFrame()));
  connect(m_gui->deleteCF, SIGNAL(clicked()),
          this,            SLOT(deleteActiveCountingFrame()));

  connect(m_gui->countingFrames, SIGNAL(clicked(QModelIndex)),
          this,                  SLOT(updateUI(QModelIndex)));

  connect(m_gui->leftMargin, SIGNAL(valueChanged(double)),
          this,              SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->topMargin, SIGNAL(valueChanged(double)),
          this,             SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->frontMargin, SIGNAL(valueChanged(double)),
          this,               SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->rightMargin, SIGNAL(valueChanged(double)),
          this,               SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->bottomMargin, SIGNAL(valueChanged(double)),
          this,                SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->backMargin, SIGNAL(valueChanged(double)),
          this,              SLOT(updateActiveCountingFrameMargins()));

  connect(m_gui->useCategoryConstraint, SIGNAL(toggled(bool)),
          this,                         SLOT(enableCategoryConstraints(bool)));
  connect(m_gui->categorySelector, SIGNAL(activated(QModelIndex)),
          this,                    SLOT(applyCategoryConstraint()));

  connect(m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this,      SLOT(onCountingFrameCreated(CountingFrame*)));

  connect(m_context.model().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
          this,                    SLOT(onSegmentationsAdded(ViewItemAdapterSList)));

  connect(getSelection().get(), SIGNAL(activeChannelChanged(ChannelAdapterPtr)),
          this,                 SLOT(onChannelChanged(ChannelAdapterPtr)));
}

//------------------------------------------------------------------------
CF::Panel::~Panel()
{
  for(auto cf: m_countingFrames)
  {
    deleteCountingFrame(cf);
  }

  delete m_cfModel;
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Counting Frame Panel Plugin";
//   qDebug() << "********************************************************";
}

//------------------------------------------------------------------------
void CF::Panel::reset()
{
  m_gui->countingFrameDescription->clear();

  m_gui->countingFrames->setModel(nullptr);

  m_gui->createCF->setEnabled(false);
  m_gui->deleteCF->setEnabled(false);
  m_gui->resetCF ->setEnabled(false);
  m_gui->exportCF->setEnabled(false);

  for(auto cf: m_countingFrames)
  {
    deleteCountingFrame(cf);
  }
}

//------------------------------------------------------------------------
void CF::Panel::deleteCountingFrame(CountingFrame *cf)
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

  updateSegmentationExtensions();

  updateSegmentationRepresentations();
}

//------------------------------------------------------------------------
void CF::Panel::applyCategoryConstraint()
{
  if(!m_activeCF) return;

  if (m_gui->useCategoryConstraint->isChecked())
  {
    auto categoryIndex = m_gui->categorySelector->currentModelIndex();
    if (categoryIndex.isValid())
    {
      auto item = itemAdapter(categoryIndex);
      Q_ASSERT(isCategory(item));

      auto category = toCategoryAdapterPtr(item);

      m_activeCF->setCategoryConstraint(category->classificationName());
    }
  }
  else
  {
    m_activeCF->setCategoryConstraint("");
  }
}

//------------------------------------------------------------------------
void CF::Panel::enableCategoryConstraints(bool enable)
{
  m_gui->categorySelector->setEnabled(enable);

  applyCategoryConstraint();
}

//------------------------------------------------------------------------
void CF::Panel::updateUI(QModelIndex index)
{
  bool validCF = !m_countingFrames.isEmpty() && index.isValid();

  if (validCF)
  {
    CountingFrame *cf;
    cf = m_cfModel->countingFrame(index);
    Q_ASSERT(cf);

    showInfo(cf);
  }
  else
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
void CF::Panel::createCountingFrame()
{
  if (!m_pendingCFs.isEmpty()) return;

  CFTypeSelectorDialog cfSelector(m_context.model(), this);

  if (cfSelector.exec())
  {
    CFType type        = cfSelector.type();
    QString constraint = cfSelector.categoryConstraint();

    auto channel = cfSelector.channel();
    Q_ASSERT(channel);

    auto extensions = channel->extensions();
    if (!extensions->hasExtension(CountingFrameExtension::TYPE))
    {
      extensions->add(m_manager->createExtension(m_context.scheduler()));
    }

    auto spacing = channel->output()->spacing();

    Nm inclusion[3];
    Nm exclusion[3];
    for(int i = 0; i < 3; ++i)
    {
      inclusion[i] = exclusion[i] = spacing[i]/2;
    }

    auto extension = retrieveExtension<CountingFrameExtension>(extensions);

    WaitingCursor cursor;
    extension->createCountingFrame(type, inclusion, exclusion, constraint);
  }
}

//------------------------------------------------------------------------
void CF::Panel::resetActiveCountingFrame()
{
  if (m_activeCF)
  {
    auto channel       = m_activeCF->channel();
    auto segmentations = QueryContents::segmentationsOnChannelSample(channel);

    ComputeOptimalMarginsSPtr task(new ComputeOptimalMarginsTask(channel, segmentations, m_context.scheduler()));

    connect(task.get(), SIGNAL(finished()),
            this,       SLOT(onMarginsComputed()));

    m_pendingCFs << PendingCF(m_activeCF, task);

    Task::submit(task);
  }
}

//------------------------------------------------------------------------
void CF::Panel::updateActiveCountingFrameMargins()
{
  if (!m_activeCF) return;

  Nm inclusion[3];
  Nm exclusion[3];

  inclusionMargins(inclusion);
  exclusionMargins(exclusion);

  m_activeCF->setMargins(inclusion, exclusion);
}

//------------------------------------------------------------------------
void CF::Panel::deleteActiveCountingFrame()
{
  if (!m_activeCF) return;

  deleteCountingFrame(m_activeCF);
}

//------------------------------------------------------------------------
void CF::Panel::onChannelChanged(ChannelAdapterPtr channel)
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
    {
      lenght[i] = bounds[2*i+1]-bounds[2*i];
    }

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
void CF::Panel::showInfo(CountingFrame* activeCF)
{
  if (!activeCF || !getActiveChannel()) return;

  m_activeCF = activeCF;

  int row = m_manager->countingFrames().indexOf(activeCF);

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
  m_gui->categorySelector->blockSignals(true);
  if (applyCaregoryConstraint)
  {
    m_gui->categorySelector->setCurrentModelIndex(findCategoryIndex(activeCF->categoryConstraint()));
  }
  m_gui->categorySelector->blockSignals(false);

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
QModelIndex CF::Panel::findCategoryIndex(const QString& classificationName)
{
  auto model    = m_context.model();
  auto category = model->classification()->category(classificationName);

  return model->categoryIndex(category);
}

//------------------------------------------------------------------------
void CF::Panel::updateSegmentationRepresentations()
{
  auto model         = m_context.model();
  auto segmentations = toRawList<ViewItemAdapter>(model->segmentations());

  getViewState().invalidateRepresentationColors(segmentations);
}

//------------------------------------------------------------------------
void CF::Panel::updateSegmentationExtensions()
{
  for (auto seg: m_context.model()->segmentations())
  {
    auto extensions = seg->extensions();

    if(extensions->hasExtension(StereologicalInclusion::TYPE))
    {
      auto extension = retrieveExtension<StereologicalInclusion>(extensions);
      if(!extension->hasCountingFrames())
      {
        extensions->remove(extension);
      }
    }
  }
}

//------------------------------------------------------------------------
void CF::Panel::changeUnitMode(bool useSlices)
{
  m_useSlices = useSlices;

  if (useSlices)
  {
    m_gui->frontMargin->setSuffix("");
    m_gui->backMargin ->setSuffix("");
  }
  else
  {
    m_gui->frontMargin->setSuffix(" nm");
    m_gui->backMargin ->setSuffix(" nm");
  }

  showInfo(m_activeCF);
}

//------------------------------------------------------------------------
void CF::Panel::reportProgess(int progress)
{
  QIcon icon(":/create-cf.svg");

  auto size = m_gui->createCF->size();
  auto original = icon.pixmap(size);
  auto inverted = icon.pixmap(size, QIcon::Disabled);

//   QPainter painter(&pixmap);
//   QRect rect = pixmap.rect();
//   QLinearGradient gradient(rect.bottomLeft() - QPoint(0,progress), rect.topLeft());
//   gradient.setColorAt(0, QColor(0,0,0,255));
//   gradient.setColorAt(1, QColor(0,0,0,50));
//   painter.fillRect(rect, gradient);
//   painter.fillRect(rect, QColor(125,125,125,125));

  auto originalImage = original.toImage();
  auto invertedImage = inverted.toImage();

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
void CF::Panel::computeOptimalMargins(ChannelAdapterPtr channel,
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
    auto extension = retrieveOrCreateExtension<EdgeDistance>(segmentation->extensions());

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

//------------------------------------------------------------------------
void CF::Panel::inclusionMargins(double values[3])
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
void CF::Panel::exclusionMargins(double values[3])
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
void CF::Panel::onMarginsComputed()
{
  auto task = dynamic_cast<TaskPtr>(sender());
  PendingCF                 pendingCF;
  ComputeOptimalMarginsSPtr optimalMargins;

  for(auto cf : m_pendingCFs)
  {
    if (cf.Task.get() == task)
    {
      pendingCF      = cf;
      optimalMargins = cf.Task;
      m_pendingCFs.removeOne(cf);
      break;
    }
  }

  Q_ASSERT(pendingCF.Task);

  if(!pendingCF.Task->isAborted())
  {
    Nm inclusion[3] = {0, 0, 0};
    Nm exclusion[3] = {0, 0 ,0};

    optimalMargins->inclusion(inclusion);
    optimalMargins->exclusion(exclusion);

    pendingCF.CF->setMargins(inclusion, exclusion);
    pendingCF.CF->apply();
  }
}


//------------------------------------------------------------------------
void CF::Panel::onCountingFrameCreated(CountingFrame* cf)
{
  connect(cf,   SIGNAL(modified(CountingFrame*)),
          this, SLOT(showInfo(CountingFrame*)));
  connect(cf,   SIGNAL(applied(CountingFrame*)),
          this, SLOT(onCountingFrameApplied(CountingFrame*)));

  cf->apply();

  m_countingFrames << cf;

  updateTable();

  m_activeCF = cf; // To make applyCategoryConstraint work

  updateUI(m_cfModel->index(m_cfModel->rowCount() - 1, 0));
}

//------------------------------------------------------------------------
void CF::Panel::onCountingFrameApplied(CountingFrame *cf)
{
  updateSegmentationRepresentations();
}

//------------------------------------------------------------------------
void CF::Panel::onSegmentationsAdded(ViewItemAdapterSList items)
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
void CF::Panel::updateTable()
{
  m_gui->countingFrames->setModel(nullptr);
  m_gui->countingFrames->setModel(m_cfModel);
}

//------------------------------------------------------------------------
void ESPINA::CF::Panel::exportCountingFramesData()
{
  auto title      = tr("Export Counting Frames data");
  auto suggestion = tr("CF_Data_%1.txt").arg(getContext().viewState().selection()->activeChannel()->data().toString());
  auto formats    = SupportedFormats().addTxtFormat().addCSVFormat().addExcelFormat();

  auto fileName = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".txt", suggestion, this);

  if(!fileName.isEmpty())
  {
    QStringList splittedName = fileName.split(".");
    QString extension = splittedName.last().toUpper().remove(' ');

    QStringList validFileExtensions;
    validFileExtensions << "TXT" << "CSV" << "XLS";

    if(validFileExtensions.contains(extension))
    {
      try
      {
        if(QString("TXT") == extension)
        {
          exportAsText(fileName);
        }

        if(QString("CSV") == extension)
        {
          exportAsCSV(fileName);
        }

        if(QString("XLS") == extension)
        {
          exportAsXLS(fileName);
        }
      }
      catch(const EspinaException &e)
      {
        auto message = tr("Couldn't export data to %1. ").arg(fileName);
        DefaultDialogs::InformationMessage(message, title, e.what(), this);
      }
    }
    else
    {
      auto message = tr("Couldn't export data to %1. Format not supported.").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, "", this);
    }
  }
}

//------------------------------------------------------------------------
void ESPINA::CF::Panel::exportAsText(const QString& fileName) const
{
  QFile file{fileName};
  if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
  {
    auto message = tr("Couldn't create file %1, check file permissions.").arg(fileName);
    auto details = tr("CF::Panel::exportAsText() ->") + message;

    throw EspinaException(message, details);
  }

  QTextStream out(&file);
  for(auto cf: m_countingFrames)
  {
    out << cf->description();
    out << '\n';
  }

  out.flush();
  file.close();

  if(file.error() != QFile::NoError)
  {
    auto message = tr("Couldn't save file '%1'. Cause: %2").arg(fileName.split('/').last()).arg(file.errorString());
    auto details = tr("CF::Panel::exportAsText() ->") + message;

    throw EspinaException(message, details);
  }
}

//------------------------------------------------------------------------
void ESPINA::CF::Panel::exportAsCSV(const QString& fileName) const
{
  QFile file{fileName};
  if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
  {
    auto message = tr("Couldn't create file %1, check file permissions.").arg(fileName);
    auto details = tr("CF::Panel::exportAsCSV() ->") + message;

    throw EspinaException(message, details);
  }

  QTextStream out(&file);

  QStringList columnLabels;
  columnLabels << "CF id" << "Type" << "Constraint" << "Stack"
               << "Total Volume (voxels)" << "Total Volume (nm^3)"
               << "Inclusion Volume (voxels)" << "Inclusion Volume (nm^3)"
               << "Exclusion Volume (voxels)" << "Exclusion Volume (nm^3)"
               << "Left Margin (nm)" << "Top Margin (nm)" << "Right Margin (nm)" << "Bottom Margin (nm)"
               << "Front Margin (nm)" << "Front Margin (slices)" << "Back Margin (nm)" << "Back Margin (slices)";

  for(int pos = 0; pos < columnLabels.size(); ++pos)
  {
    if(pos)
    {
      out << ",";
    }
    out << columnLabels.at(pos);
  }
  out << "\n";

  for(auto cf: m_countingFrames)
  {
    auto channel              = cf->extension()->extendedItem();
    auto spacing              = channel->output()->spacing();
    Nm   voxelVol             = spacing[0]*spacing[1]*spacing[2];
    int  totalVoxelVolume     = cf->totalVolume()     / voxelVol;
    int  inclusionVoxelVolume = cf->inclusionVolume() / voxelVol;
    int  exclusionVoxelVolume = cf->exclusionVolume() / voxelVol;
    int  frontSl              = int(cf->front()/spacing[2]);
    int  backSl               = int(cf->back()/spacing[2]);
    auto constraint           = cf->categoryConstraint().isEmpty() ? "None (Global)" : cf->categoryConstraint();

    out << cf->id()                                          << ",";
    out << cf->typeName()                                    << ",";
    out << constraint                                        << ",";
    out << channel->name()                                   << ",";
    out << QString("%1").arg(totalVoxelVolume)               << ",";
    out << QString("%1").arg(cf->totalVolume(), 0,'f',2)     << ",";
    out << QString("%1").arg(inclusionVoxelVolume)           << ",";
    out << QString("%1").arg(cf->inclusionVolume(), 0,'f',2) << ",";
    out << QString("%1").arg(exclusionVoxelVolume)           << ",";
    out << QString("%1").arg(cf->exclusionVolume(), 0,'f',2) << ",";
    out << QString("%1").arg(cf->left())                     << ",";
    out << QString("%1").arg(cf->top())                      << ",";
    out << QString("%1").arg(cf->right())                    << ",";
    out << QString("%1").arg(cf->bottom())                   << ",";
    out << QString("%1").arg(cf->front())                    << ",";
    out << QString("%1").arg(frontSl)                        << ",";
    out << QString("%1").arg(cf->back())                     << ",";
    out << QString("%1").arg(backSl)                         << "\n";
  }
  file.close();

  if(file.error() != QFile::NoError)
  {
    auto message = tr("Couldn't save file '%1'. Cause: %2").arg(fileName.split('/').last()).arg(file.errorString());
    auto details = tr("CF::Panel::exportAsCSV() ->") + message;

    throw EspinaException(message, details);
  }
}

//------------------------------------------------------------------------
void ESPINA::CF::Panel::exportAsXLS(const QString& fileName) const
{
  workbook wb;

  worksheet *ws = wb.sheet("Counting Frames");

  QStringList columnLabels;
  columnLabels << "CF id" << "Type" << "Constraint" << "Stack"
               << "Total Volume (voxels)" << "Total Volume (nm^3)"
               << "Inclusion Volume (voxels)" << "Inclusion Volume (nm^3)"
               << "Exclusion Volume (voxels)" << "Exclusion Volume (nm^3)"
               << "Left Margin (nm)" << "Top Margin (nm)" << "Right Margin (nm)" << "Bottom Margin (nm)"
               << "Front Margin (nm)" << "Front Margin (slices)" << "Back Margin (nm)" << "Back Margin (slices)";

  for(int pos = 0; pos < columnLabels.size(); ++pos)
  {
    createCell(ws, 0, pos, columnLabels.at(pos));
  }

  int row = 0;
  for(auto cf: m_countingFrames)
  {
    ++row;
    int column = 0;

    auto channel              = cf->extension()->extendedItem();
    auto spacing              = channel->output()->spacing();
    Nm   voxelVol             = spacing[0]*spacing[1]*spacing[2];
    int  totalVoxelVolume     = cf->totalVolume()     / voxelVol;
    int  inclusionVoxelVolume = cf->inclusionVolume() / voxelVol;
    int  exclusionVoxelVolume = cf->exclusionVolume() / voxelVol;
    int  frontSl              = int(cf->front()/spacing[2]);
    int  backSl               = int(cf->back()/spacing[2]);
    auto constraint           = cf->categoryConstraint().isEmpty() ? "None (Global)" : cf->categoryConstraint();

    createCell(ws, row, column++, cf->id());
    createCell(ws, row, column++, cf->typeName());
    createCell(ws, row, column++, constraint);
    createCell(ws, row, column++, channel->name());
    createCell(ws, row, column++, totalVoxelVolume);
    createCell(ws, row, column++, cf->totalVolume());
    createCell(ws, row, column++, inclusionVoxelVolume);
    createCell(ws, row, column++, cf->inclusionVolume());
    createCell(ws, row, column++, exclusionVoxelVolume);
    createCell(ws, row, column++, cf->exclusionVolume());
    createCell(ws, row, column++, cf->left());
    createCell(ws, row, column++, cf->top());
    createCell(ws, row, column++, cf->right());
    createCell(ws, row, column++, cf->bottom());
    createCell(ws, row, column++, cf->front());
    createCell(ws, row, column++, frontSl);
    createCell(ws, row, column++, cf->back());
    createCell(ws, row, column++, backSl);
  }

  auto result = wb.Dump(fileName.toStdString());

  if(result != NO_ERRORS)
  {
    auto what    = tr("exportToXLS: can't save file '%1'.").arg(fileName);
    auto details = tr("Cause of failure: %1").arg(result == FILE_ERROR ? "file error" : "general error");

    throw EspinaException(what, details);
  }
}

//------------------------------------------------------------------------
void CF::Panel::applyCountingFrames(SegmentationAdapterSList segmentations)
{
  for (auto segmentation : segmentations)
  {
    auto segmentationExtensions = segmentation->extensions();
    auto sterologicalExtension  = retrieveOrCreateExtension<StereologicalInclusion>(segmentationExtensions);

    auto samples = QueryAdapter::samples(segmentation);
    Q_ASSERT(samples.size() == 1);

    for (auto channel : QueryAdapter::channels(samples[0]))
    {
      auto channelExtensions = channel->readOnlyExtensions();
      if (channelExtensions->hasExtension(CountingFrameExtension::TYPE))
      {
        auto cfExtension = retrieveExtension<CountingFrameExtension>(channelExtensions);

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
void CF::Panel::deleteCountingFrames()
{
  for(auto cf: m_countingFrames)
  {
    deleteCountingFrame(cf);
  }
}
