/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
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
#include "Dialogs/TypeDialog.h"
#include "Extensions/CountingFrameExtension.h"

#include "ui_Panel.h"

#include <Support/ViewManager.h>
#include <Core/Analysis/Channel.h>

#include <QFileDialog>

using namespace EspINA;
using namespace EspINA::CF;

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

  QString tooltip("%1 Face: <br><br> &nbsp; <img src=':/%1.png' width=125 height=94> &nbsp; <br>");
  leftMargin  ->setToolTip(tooltip.arg("Left"));
  rightMargin ->setToolTip(tooltip.arg("Right"));
  topMargin   ->setToolTip(tooltip.arg("Top"));
  bottomMargin->setToolTip(tooltip.arg("Bottom"));
  frontMargin ->setToolTip(tooltip.arg("Front"));
  backMargin  ->setToolTip(tooltip.arg("Back"));
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

private:
  CountingFrame *countingFrame(const QModelIndex &index) const
  { return m_manager->countingFrames()[index.row()]; }

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
      return Qt::Checked;
    }
  } else if (1 == c && Qt::DisplayRole == role)
  {
      return cf->name();
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

    cf->setId(value.toString());

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
             ModelAdapterSPtr      model,
             ViewManagerSPtr       viewManager,
             QWidget              *parent = nullptr)
: DockWidget(parent)
, m_manager(manager)
, m_model(model)
, m_viewManager(viewManager)
, m_gui(new GUI())
, m_cfModel(new CFModel(m_manager))
, m_useSlices(false)
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
  connect(m_gui->deleteCF, SIGNAL(clicked()),
          this, SLOT(deleteActiveCountingFrame()));

  connect(m_gui->countingFrames, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateUI(QModelIndex)));

  connect(m_gui->leftMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->topMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->frontMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->rightMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->bottomMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));
  connect(m_gui->backMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateActiveCountingFrameMargins()));

  connect(m_gui->useCategoryConstraint, SIGNAL(toggled(bool)),
          this, SLOT(enableCategoryConstraints(bool)));
  connect(m_gui->categorySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(applyCategoryConstraint()));

  connect(m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(onCountingFrameCreated(CountingFrame*)));

  connect(m_viewManager.get(), SIGNAL(activeChannelChanged(ChannelAdapterPtr)),
          this, SLOT(onChannelChanged(ChannelAdapterPtr)));
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
  if (cf == m_activeCF) m_activeCF = nullptr;

  m_countingFrames.removeOne(cf);

  m_viewManager->removeWidget(cf);

  m_manager->deleteCountingFrame(cf);

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

      // TODO
//       auto category = m_model->smartPointer(categoryPtr(item));
//       m_activeCF->setCategoryConstraint(category);
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
    CountingFrame *cf = m_countingFrames.value(index.row(), nullptr);
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
    m_gui->backMargin ->setValue(0);

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
  auto channel = m_viewManager->activeChannel();
  Q_ASSERT(channel);

  Nm inclusion[3];
  Nm exclusion[3];

  computeOptimalMargins(channel, inclusion, exclusion);
  memset(exclusion, 0, 3*sizeof(Nm));

  TypeDialog typeSelector(this);

  if (typeSelector.exec())
  {
    if (ADAPTIVE == typeSelector.type())
      m_manager->createAdaptiveCF(channel, inclusion, exclusion);
    else
      m_manager->createRectangularCF(channel, inclusion, exclusion);
  }
//
//   updateSegmentations();
}

//------------------------------------------------------------------------
void Panel::resetActiveCountingFrame()
{
  if (m_activeCF)
  {
    Nm inclusion[3];
    Nm exclusion[3];

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_activeCF->setMargins(inclusion, exclusion);
    //computeOptimalMargins(m_activeCF->extension()->channel(), inclusion, exclusion);
    memset(exclusion, 0, 3*sizeof(Nm));

    m_activeCF->setMargins(inclusion, exclusion);

    QApplication::restoreOverrideCursor();
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
  m_gui->categorySelector->setModel(m_model.get());
  m_gui->categorySelector->setRootModelIndex(m_model->classificationRoot());

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
void Panel::showInfo(CountingFrame* cf)
{
  if (!cf || !m_viewManager->activeChannel())
    return;

  m_activeCF = cf;

  int row = m_countingFrames.indexOf(cf);


  auto selectionModel = m_gui->countingFrames->selectionModel();
  auto index = m_cfModel->index(row, 0);
  selectionModel->select(index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);

  m_gui->leftMargin  ->blockSignals(true);
  m_gui->topMargin   ->blockSignals(true);
  m_gui->frontMargin ->blockSignals(true);
  m_gui->rightMargin ->blockSignals(true);
  m_gui->bottomMargin->blockSignals(true);
  m_gui->backMargin  ->blockSignals(true);

  NmVector3 spacing{1., 1., 1.};
  if (m_useSlices)
  {
    auto activeChannel = m_viewManager->activeChannel();
    spacing = activeChannel->output()->spacing();
  }

  m_gui->leftMargin  ->setValue(cf->left());
  m_gui->topMargin   ->setValue(cf->top() );
  m_gui->frontMargin ->setValue(vtkMath::Round(cf->front()/spacing[2]));
  m_gui->rightMargin ->setValue(cf->right() );
  m_gui->bottomMargin->setValue(cf->bottom());
  m_gui->backMargin ->setValue(vtkMath::Round(cf->back()/spacing[2]));

  m_gui->leftMargin  ->blockSignals(false);
  m_gui->topMargin   ->blockSignals(false);
  m_gui->frontMargin ->blockSignals(false);
  m_gui->rightMargin ->blockSignals(false);
  m_gui->bottomMargin->blockSignals(false);
  m_gui->backMargin  ->blockSignals(false);

  m_gui->useCategoryConstraint->setChecked(nullptr != cf->categoryConstraint());

  m_gui->countingFrameDescription->setText(cf->description());
}

//------------------------------------------------------------------------
void Panel::updateSegmentations()
{
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}


//------------------------------------------------------------------------
void Panel::saveActiveCountingFrameDescription()
{
  QString title    = tr("Save Counting Frame Description");
  QString fileExt  = tr("Text File (*.txt)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (!fileName.isEmpty())
  {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly |  QIODevice::Text);
    QTextStream out(&file);
    out << m_gui->countingFrameDescription->toPlainText();
    file.close();
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
void Panel::computeOptimalMargins(ChannelAdapterPtr channel,
                                  Nm inclusion[3],
                                  Nm exclusion[3])
{
//   double spacing[3];
//   channel->volume()->spacing(spacing);
//
//   const Nm delta[3] = { 0.5*spacing[0], 0.5*spacing[1], 0.5*spacing[2] };

  memset(inclusion, 0, 3*sizeof(Nm));
  memset(exclusion, 0, 3*sizeof(Nm));

//   ModelItemSList items = channel->relatedItems(EspINA::RELATION_OUT, Channel::LINK);
//   SegmentationSList channelSegs;
//   foreach(ModelItemSPtr item, items)
//   {
//     if (EspINA::SEGMENTATION == item->type())
//       channelSegs << segmentationPtr(item);
//   }
//
//   foreach(SegmentationSPtr seg, channelSegs)
//   {
//     Segmentation::InformationExtension ext = seg->informationExtension(EdgeDistanceID);
//     EdgeDistancePtr marginExt = dynamic_cast<EdgeDistancePtr>(ext);
//     if (marginExt)
//     {
//       Nm dist2Margin[6];
//       marginExt->edgeDistance(dist2Margin);
//
//       Nm segBounds[6];
//       Nm segSpacing[3];
//       SegmentationVolumeSPtr volume = segmentationVolume(seg->output());
//       volume->spacing(segSpacing);
//       volume->bounds(segBounds);
//
//       for (int i=0; i < 3; i++)
//       {
//         double shift = i < 2? 0.5:-0.5;
//         Nm length = segBounds[2*i+1] - segBounds[2*i];
//         if (dist2Margin[2*i] < delta[i])
//           inclusion[i] = (vtkMath::Round(std::max(length, inclusion[i])/spacing[i]-shift)+shift)*spacing[i];
// //         if (dist2Margin[2*i+1] < delta[i])
// //           exclusion[i] = std::max(length, exclusion[i]);
//       }
//     }
//   }
// //   qDebug() << "Inclusion:" << inclusion[0] << inclusion[1] << inclusion[2];
// //   qDebug() << "Exclusion:" << exclusion[0] << exclusion[1] << exclusion[2];
}

//------------------------------------------------------------------------
void Panel::inclusionMargins(double values[3])
{
  values[0] = m_gui->leftMargin ->value();
  values[1] = m_gui->topMargin  ->value();
  values[2] = m_gui->frontMargin->value();
}

//------------------------------------------------------------------------
void Panel::exclusionMargins(double values[3])
{
  values[0] = m_gui->rightMargin ->value();
  values[1] = m_gui->bottomMargin->value();
  values[2] = m_gui->backMargin  ->value();
}

//------------------------------------------------------------------------
void Panel::onCountingFrameCreated(CountingFrame* cf)
{
  connect(cf,   SIGNAL(modified(CountingFrame*)),
          this, SLOT(showInfo(CountingFrame*)));

  m_countingFrames << cf;

  updateTable();

  m_viewManager->addWidget(cf);

  m_activeCF = cf; // To make applyCategoryConstraint work

  updateUI(m_cfModel->index(m_cfModel->rowCount() - 1, 0));
}

void Panel::updateTable()
{
  m_gui->countingFrames->setModel(nullptr);
  m_gui->countingFrames->setModel(m_cfModel);
}
