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

#include <common/model/EspinaModel.h>
#include <model/Segmentation.h>

#include <QAction>
#include <QComboBox>
#include <QIcon>
#include <QTreeView>
#include <EspinaCore.h>
#include <selection/PixelSelector.h>
#include <undo/RemoveSegmentation.h>
#include <iostream>

//----------------------------------------------------------------------------
MainToolBar::MainToolBar(QSharedPointer<EspinaModel> model, QWidget* parent)
: QToolBar(parent)
{
  setObjectName("MainToolBar");
  setWindowTitle("Main Tool Bar");

  m_toggleSegVisibility = addAction(//showIcon,
				  tr("Toggle Segmentations Visibility"));

  m_toggleSegVisibility->setShortcut(QString("Space"));
  m_toggleSegVisibility->setCheckable(true);
  m_toggleSegVisibility->setChecked(true);
  setShowSegmentations(true);
  connect(m_toggleSegVisibility,SIGNAL(triggered(bool)),
	  this,SLOT(setShowSegmentations(bool)));


   // User selected Taxonomy Selection List
  m_taxonomyView = new QTreeView(this);
  m_taxonomyView->setHeaderHidden(true);

  m_taxonomySelector = new QComboBox(this);
  m_taxonomySelector->setView(m_taxonomyView); //Brutal!
  m_taxonomySelector->setModel(model.data());
  m_taxonomySelector->setRootModelIndex(model->taxonomyRoot());
  connect(model.data(),SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this,SLOT(updateTaxonomy(QModelIndex,QModelIndex)));

  m_taxonomySelector->setMinimumWidth(160);
  m_taxonomySelector->setToolTip( tr("Type of new segmentation") );
  m_taxonomySelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  addWidget(m_taxonomySelector);

  connect(m_taxonomyView, SIGNAL(entered(QModelIndex)),
          this, SLOT(setActiveTaxonomy(QModelIndex)));
  connect(m_taxonomySelector,SIGNAL(currentIndexChanged(QString)),
          this, SLOT(setActiveTaxonomy(QString)));

  m_selector = new PixelSelector();
  m_selector->setMultiSelection(false);
  m_selector->setSelectable(SelectionHandler::EspINA_Segmentation);
  connect(m_selector, SIGNAL(selectionAborted()),
	  this, SLOT(abortSelection()));
  connect(m_selector, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(removeSelectedSegmentation(SelectionHandler::MultiSelection)));

  m_removeSegmentation = addAction(QIcon(":/espina/removeSeg.svg"),
					  tr("Remove Segmentation"));
  m_removeSegmentation->setCheckable(true);
  connect(m_removeSegmentation, SIGNAL(toggled(bool)),
	  this, SLOT(removeSegmentation(bool)));
}

//----------------------------------------------------------------------------
void MainToolBar::setShowSegmentations(bool visible)
{
  if (visible)
    m_toggleSegVisibility->setIcon(QIcon(":/espina/show_all.svg"));
  else
    m_toggleSegVisibility->setIcon(QIcon(":/espina/hide_all.svg"));

  //EspinaCore::instance()->model()->relationships()->write(std::cout, RelationshipGraph::GRAPHVIZ);

  emit showSegmentations(visible);
}

//----------------------------------------------------------------------------
void MainToolBar::setActiveTaxonomy(QModelIndex index)
{
  if (!index.isValid())
    return;

  ModelItem *item = static_cast<ModelItem *>(index.internalPointer());
  Q_ASSERT(item->type() == ModelItem::TAXONOMY);
  TaxonomyNode *tax = dynamic_cast<TaxonomyNode *>(item);
  Q_ASSERT(tax);
  SelectionManager::instance()->setActiveTaxonomy(tax);
}

//----------------------------------------------------------------------------
void MainToolBar::setActiveTaxonomy(QString taxonomy)
{
  if (taxonomy.isEmpty())
    return;

  TaxonomyNode *tax = EspinaCore::instance()->model()->taxonomy()->element(taxonomy);
  if (tax)
    SelectionManager::instance()->setActiveTaxonomy(tax);
}


//----------------------------------------------------------------------------
void MainToolBar::updateTaxonomy(QModelIndex left, QModelIndex right)
{
  if (left == m_taxonomySelector->view()->rootIndex())
  {
    m_taxonomySelector->setCurrentIndex(0);
    setActiveTaxonomy(left.child(0,0));
  }
  m_taxonomyView->expandAll();
}

//----------------------------------------------------------------------------
void MainToolBar::removeSegmentation(bool active)
{
  if (active)
  {
    SelectionManager::instance()->setSelectionHandler(m_selector);
  }else
  {
    SelectionManager::instance()->setSelectionHandler(NULL);
  }
}

//----------------------------------------------------------------------------
void MainToolBar::removeSelectedSegmentation(SelectionHandler::MultiSelection msel)
{
  if (msel.size() != 1)
    return;

  SelectionHandler::Selelection element = msel.first();

  SelectableItem *input = element.second;
  Q_ASSERT(ModelItem::SEGMENTATION == input->type());
  QList<Segmentation *> removedSegs;
  removedSegs << dynamic_cast<Segmentation *>(input);
  QSharedPointer<QUndoStack> undoStack = EspinaCore::instance()->undoStack();
  undoStack->beginMacro(tr("Delete Segmentation"));
  undoStack->push(new RemoveSegmentation(removedSegs));
  undoStack->endMacro();
}

//----------------------------------------------------------------------------
void MainToolBar::abortSelection()
{
  m_removeSegmentation->blockSignals(true);
  m_removeSegmentation->setChecked(false);
  m_removeSegmentation->blockSignals(false);
}
