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
#include <GUI/Pickers/PixelSelector.h>
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
, m_rulerTool  (new RulerTool(m_viewManager))
{
  setObjectName("MainToolBar");

  setWindowTitle(tr("Main Tool Bar"));

  // Segmentation visibility
  m_toggleSegVisibility = addAction(tr("Toggle Segmentation Visibility"));

  m_toggleSegVisibility->setShortcut(QString("Space"));
  m_toggleSegVisibility->setCheckable(true);
  m_toggleSegVisibility->setChecked(true);
  setShowSegmentations(true);
  connect(m_toggleSegVisibility,SIGNAL(triggered(bool)),
          this,SLOT(setShowSegmentations(bool)));

  // Cross-hair visibility
  m_toggleCrosshair = addAction(QIcon(":/espina/hide_planes.svg"),
                                tr("Toggle Crosshair"));
  m_toggleCrosshair->setCheckable(true);
  m_toggleCrosshair->setShortcut(QKeySequence("C"));
  connect(m_toggleCrosshair, SIGNAL(toggled(bool)),
          this, SLOT(toggleCrosshair(bool)));

  // Taxonomy selection
  m_categorySelector = new QComboTreeView(this);
  m_categorySelector->setModel(model);
  m_categorySelector->setRootModelIndex(model->taxonomyRoot());
  m_categorySelector->setMinimumHeight(28);
  connect(m_categorySelector,SIGNAL(activated(QModelIndex)),
          this, SLOT(setActiveTaxonomy(QModelIndex)));
  connect(m_model, SIGNAL(taxonomyAdded(TaxonomySPtr)),
          this,  SLOT(updateTaxonomy(TaxonomySPtr)));
  connect(m_model, SIGNAL(modelReset()),
          this, SLOT(resetRootItem()));
  m_categorySelector->setToolTip( tr("Active Category") );

  addWidget(m_categorySelector);

  // Segmentation Remover
  connect(m_segRemover.get(), SIGNAL(removalAborted()),
          this, SLOT(abortRemoval()));
  connect(m_segRemover.get(), SIGNAL(removeSegmentation(SegmentationPtr)),
          this, SLOT(removeSegmentation(SegmentationPtr)));

  m_removeSegmentation = addAction(QIcon(":/espina/removeSeg.svg"),
                                   tr("Delete Segmentation"));
  m_removeSegmentation->setCheckable(true);
  connect(m_removeSegmentation, SIGNAL(toggled(bool)),
          this, SLOT(removeSegmentation(bool)));

  // Distance Tool
  m_measureButton = addAction(QIcon(":/espina/measure.png"),
                                tr("Measure Distance"));
  m_measureButton->setCheckable(true);
  m_measureButton->setShortcut(QKeySequence("M"));
  connect(m_measureButton, SIGNAL(toggled(bool)),
          this, SLOT(toggleMeasureTool(bool)));
  connect(m_measureTool.get(), SIGNAL(stopMeasuring()),
          this,                SLOT(abortOperation()));

  // Ruler toogle button
  m_rulerButton = addAction(QIcon(":/espina/measure3D.png"),
                            tr("Measure Selection"));
  m_rulerButton->setCheckable(true);
  m_rulerButton->setShortcut(QKeySequence("R"));
  connect(m_rulerButton, SIGNAL(toggled(bool)),
          this, SLOT(toggleRuler(bool)));
}

//----------------------------------------------------------------------------
MainToolBar::~MainToolBar()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Main ToolbBar";
//   qDebug() << "********************************************************";
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
void MainToolBar::resetToolbar()
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
    // avoid selecting SAS as the active taxonomy when updating
    if (taxonomy->elements().first().get()->data().toString().compare(QString("SAS")) == 0 &&
        taxonomy->elements().size() > 1)
    {
      m_categorySelector->setCurrentIndex(1);
      m_viewManager->setActiveTaxonomy(taxonomy->elements().at(1).get());
    }
    else
    {
      m_categorySelector->setCurrentIndex(0);
      m_viewManager->setActiveTaxonomy(taxonomy->elements().first().get());
    }
  }
}

//----------------------------------------------------------------------------
void MainToolBar::removeSegmentation(bool active)
{
  if (active)
  {
    m_viewManager->setActiveTool(m_segRemover);
    m_undoIndex = m_undoStack->index();
  }
  else
  {
    m_viewManager->unsetActiveTool(m_segRemover);
    m_undoIndex = INT_MAX;
  }
}

//----------------------------------------------------------------------------
void MainToolBar::removeSegmentation(SegmentationPtr seg)
{
  m_undoStack->beginMacro(tr("Delete Segmentation"));
  m_undoStack->push(new RemoveSegmentations(seg, m_model, m_viewManager));
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
  m_categorySelector->setRootModelIndex(m_model->taxonomyRoot());
}

//----------------------------------------------------------------------------
void MainToolBar::abortOperation()
{
  if (m_measureButton->isChecked())
  {
    m_measureButton->setChecked(false);
    toggleMeasureTool(false);
  }

  if (m_removeSegmentation->isChecked())
  {
    if (m_undoIndex < m_undoStack->index())
      return;

    m_removeSegmentation->setChecked(false);
    removeSegmentation(false);
  }
}

//----------------------------------------------------------------------------
void MainToolBar::toggleRuler(bool enable)
{
  // don't inform ViewManager, as this is a passive tool
  m_rulerTool->setInUse(enable);
  m_rulerTool->setEnabled(enable);
}
