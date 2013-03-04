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

#include "Tools/SegmentationRemover/SegmentationRemover.h"
#include "Dialogs/SegmentationInspector/SegmentationInspector.h"

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Taxonomy.h>
#include <GUI/Pickers/PixelPicker.h>
#include <GUI/QtWidget/QComboTreeView.h>
#include <GUI/ViewManager.h>
#include <Undo/RemoveSegmentation.h>
#include <App/Tools/Measure/MeasureTool.h>

// Qt
#include <QAction>
#include <QComboBox>
#include <QIcon>
#include <QTreeView>
#include <QUndoStack>

using namespace EspINA;

//----------------------------------------------------------------------------
MainToolBar::MainToolBar(EspinaModel *model,
                         QUndoStack  *undoStack,
                         ViewManager *viewManager,
                         QWidget     *parent)
: IToolBar     (parent)
, m_model      (model)
, m_undoStack  (undoStack)
, m_viewManager(viewManager)
, m_segRemover (new SegmentationRemover())
, m_measureTool(new MeasureTool(m_viewManager))
{
  setObjectName("MainToolBar");

  setWindowTitle(tr("Main Tool Bar"));

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
  connect(m_model, SIGNAL(taxonomyAdded(TaxonomySPtr)),
          this,  SLOT(updateTaxonomy(TaxonomySPtr)));
  connect(m_model, SIGNAL(modelReset()),
          this, SLOT(resetRootItem()));
  m_taxonomySelector->setToolTip( tr("Type of new segmentation") );

  addWidget(m_taxonomySelector);

  // Segmentation Remover
  connect(m_segRemover.data(), SIGNAL(removalAborted()),
          this, SLOT(abortRemoval()));
  connect(m_segRemover.data(), SIGNAL(removeSegmentation(SegmentationPtr)),
          this, SLOT(removeSegmentation(SegmentationPtr)));

  m_removeSegmentation = addAction(QIcon(":/espina/removeSeg.svg"),
                                   tr("Remove Segmentation"));
  m_removeSegmentation->setCheckable(true);
  connect(m_removeSegmentation, SIGNAL(toggled(bool)),
          this, SLOT(removeSegmentation(bool)));

  // Distance Tool
  m_removeSegmentation->setCheckable(true);
  m_measureButton = addAction(QIcon(":/espina/measure.png"),
                                tr("Measure tool"));
  m_measureButton->setCheckable(true);
  m_measureButton->setShortcut(QKeySequence("M"));
  connect(m_measureButton, SIGNAL(toggled(bool)),
          this, SLOT(toggleMeasureTool(bool)));
}

//----------------------------------------------------------------------------
MainToolBar::~MainToolBar()
{
  qDebug() << "********************************************************";
  qDebug() << "              Destroying Main ToolbBar";
  qDebug() << "********************************************************";
}

//----------------------------------------------------------------------------
void MainToolBar::initToolBar(EspinaModel *model,
                              QUndoStack  *undoStack,
                              ViewManager *viewManager)
{

}


//----------------------------------------------------------------------------
void MainToolBar::setShowSegmentations(bool visible)
{
//   std::filebuf fb;
//   fb.open("/home/jpena/graph.dot", ios::out);
//   std::ostream os(&fb);
// 
//   m_model->relationships()->write(os, RelationshipGraph::GRAPHVIZ);
// 
//   fb.close();

  if (visible)
    m_toggleSegVisibility->setIcon(QIcon(":/espina/show_all.svg"));
  else
    m_toggleSegVisibility->setIcon(QIcon(":/espina/hide_all.svg"));

  emit showSegmentations(visible);
}

//----------------------------------------------------------------------------
void MainToolBar::reset()
{
  setShowSegmentations(true);
  toggleMeasureTool(false);
}

//----------------------------------------------------------------------------
void MainToolBar::setActiveTaxonomy(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ModelItemPtr item = indexPtr(index);
  Q_ASSERT(EspINA::TAXONOMY == item->type());

  TaxonomyElementPtr tax = taxonomyElementPtr(item);
  m_viewManager->setActiveTaxonomy(tax);
}

//----------------------------------------------------------------------------
void MainToolBar::updateTaxonomy(TaxonomySPtr taxonomy)
{
  if (taxonomy && !taxonomy->elements().isEmpty())
  {
    m_taxonomySelector->setCurrentIndex(0);
    m_viewManager->setActiveTaxonomy(taxonomy->elements().first().data());
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
void MainToolBar::removeSegmentation(SegmentationPtr seg)
{
  m_undoStack->beginMacro(tr("Delete Segmentation"));
  m_undoStack->push(new RemoveSegmentation(seg, m_model, m_viewManager));
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

//----------------------------------------------------------------------------
void MainToolBar::toggleMeasureTool(bool enable)
{
  if (enable)
  {
    m_viewManager->setActiveTool(m_measureTool);
    m_measureTool->setEnabled(true);
  }
  else
  {
    m_measureTool->setEnabled(false);
    m_viewManager->unsetActiveTool(m_measureTool);
  }
}

//----------------------------------------------------------------------------
void MainToolBar::resetRootItem()
{
  m_taxonomySelector->setRootModelIndex(m_model->taxonomyRoot());
}
