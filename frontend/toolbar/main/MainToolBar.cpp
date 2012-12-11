/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "MainToolBar.h"
#include "SegRemover.h"
#include <docks/SegmentationInspector.h>

// EspINA
#include "common/model/EspinaModel.h"
#include "common/model/Segmentation.h"
#include "common/model/Taxonomy.h"
#include "common/tools/PixelSelector.h"
#include "common/undo/RemoveSegmentation.h"
#include <gui/ViewManager.h>
#include <gui/QComboTreeView.h>

// Qt
#include <QAction>
#include <QComboBox>
#include <QIcon>
#include <QTreeView>
#include <QUndoStack>

//----------------------------------------------------------------------------
MainToolBar::MainToolBar(EspinaModel *model,
                         QUndoStack  *undoStack,
                         ViewManager *vm,
                         QWidget     *parent)
: QToolBar(parent)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
{
  setObjectName("MainToolBar");
  setWindowTitle("Main Tool Bar");

  m_toggleSegVisibility = addAction(tr("Toggle Segmentations Visibility"));

  m_toggleSegVisibility->setShortcut(QString("Space"));
  m_toggleSegVisibility->setCheckable(true);
  m_toggleSegVisibility->setChecked(true);
  setShowSegmentations(true);
  connect(m_toggleSegVisibility,SIGNAL(triggered(bool)),
          this,SLOT(setShowSegmentations(bool)));

  m_toggleCrosshair = addAction(QIcon(":/espina/hide_planes.svg"),
                                tr("Toggle Crosshair"));
  m_toggleCrosshair->setCheckable(true);
  m_toggleCrosshair->setShortcut(QKeySequence("C"));
  connect(m_toggleCrosshair, SIGNAL(toggled(bool)),
          this, SLOT(toggleCrosshair(bool)));

  m_taxonomySelector = new QComboTreeView(this);
  m_taxonomySelector->setModel(model);
  m_taxonomySelector->setRootModelIndex(model->taxonomyRoot());
  connect(m_taxonomySelector,SIGNAL(activated(QModelIndex)),
          this, SLOT(setActiveTaxonomy(QModelIndex)));
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this,  SLOT(updateTaxonomy(QModelIndex,QModelIndex)));
  m_taxonomySelector->setToolTip( tr("Type of new segmentation") );

  addWidget(m_taxonomySelector);


  m_segRemover = new SegRemover();
  connect(m_segRemover, SIGNAL(removalAborted()),
          this, SLOT(abortRemoval()));
  connect(m_segRemover, SIGNAL(removeSegmentation(Segmentation*)),
          this, SLOT(removeSegmentation(Segmentation*)));

  m_removeSegmentation = addAction(QIcon(":/espina/removeSeg.svg"),
                                   tr("Remove Segmentation"));
  m_removeSegmentation->setCheckable(true);
  connect(m_removeSegmentation, SIGNAL(toggled(bool)),
          this, SLOT(removeSegmentation(bool)));
}

//----------------------------------------------------------------------------
void MainToolBar::setShowSegmentations(bool visible)
{
  std::filebuf fb;
  fb.open("/home/lokifacio/graph.dot", ios::out);
  std::ostream os(&fb);

  m_model->relationships()->write(os, RelationshipGraph::GRAPHVIZ);

  fb.close();

  if (visible)
    m_toggleSegVisibility->setIcon(QIcon(":/espina/show_all.svg"));
  else
    m_toggleSegVisibility->setIcon(QIcon(":/espina/hide_all.svg"));

  emit showSegmentations(visible);
}

//----------------------------------------------------------------------------
void MainToolBar::setActiveTaxonomy(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ModelItem *item = indexPtr(index);
  Q_ASSERT(item->type() == ModelItem::TAXONOMY);
  TaxonomyElement *tax = dynamic_cast<TaxonomyElement *>(item);
  Q_ASSERT(tax);
  m_viewManager->setActiveTaxonomy(tax);
}

//----------------------------------------------------------------------------
void MainToolBar::updateTaxonomy(QModelIndex left, QModelIndex right)
{
  if (left == m_taxonomySelector->view()->rootIndex())
  {
    m_taxonomySelector->setCurrentIndex(0);
    setActiveTaxonomy(left.child(0,0));
  }
}

//----------------------------------------------------------------------------
void MainToolBar::removeSegmentation(bool active)
{
  if (active)
    m_viewManager->setActiveTool(m_segRemover);
  else
    m_viewManager->unsetActiveTool(m_segRemover);
}

//----------------------------------------------------------------------------
void MainToolBar::removeSegmentation(Segmentation *seg)
{

  //TODO 2012-10-04: Gestion de memoria...y evitar que siga abierto cuando se elimina la segementacion
  //SegmentationInspector::RemoveInspector(removedSegs);
  m_undoStack->beginMacro(tr("Delete Segmentation"));
  m_undoStack->push(new RemoveSegmentation(seg, m_model));
  m_undoStack->endMacro();
}

//----------------------------------------------------------------------------
void MainToolBar::abortRemoval()
{
  m_removeSegmentation->blockSignals(true);
  m_removeSegmentation->setChecked(false);
  m_removeSegmentation->blockSignals(false);
}

//----------------------------------------------------------------------------
void MainToolBar::toggleCrosshair(bool value)
{
  if (value)
    this->m_toggleCrosshair->setIcon(QIcon(":/espina/show_planes.svg"));
  else
    this->m_toggleCrosshair->setIcon(QIcon(":/espina/hide_planes.svg"));
  m_viewManager->showCrosshair(value);
}
