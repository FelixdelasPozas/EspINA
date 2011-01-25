/*=========================================================================

   Program: ParaView
   Module:    SegmentationToolbarActions.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "SeedGrowingSegmentation.h"
#include "selectionManager.h"
#include "objectManager.h"
#include <cache/cachedObjectBuilder.h>
#include "iPixelSelector.h"
#include <eFilter.h>

//GUI includes
#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QWidgetAction>
#include <QToolButton>
#include <QMenu>

#include "pqApplicationCore.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "pqPipelineFilter.h"
#include "vtkSMProxy.h"
#include "vtkSMInputProperty.h"

#include <QDebug>
#include "assert.h"

//-----------------------------------------------------------------------------
SeedGrowingSegmentation::SeedGrowingSegmentation(QObject* parent): ISegmentationPlugin(parent)
{
  m_selector = new PixelSelector();
  buildUI();
  connect(this,
	  SIGNAL(waitingSelection(ISelectionHandler *)),
	  SelectionManager::singleton(),
	  SLOT(setSelectionHandler(ISelectionHandler*)));
//   connect(this,
// 	  SIGNAL(selectionAborted(ISelectionHandler *)),
// 	  SelectionManager::singleton(),
// 	  SLOT(setSelectionHandler(ISelectionHandler*)));
}

void SeedGrowingSegmentation::handle(const Selection sel)
{
  
  qDebug() << "Ejecutando Plugin";
  
  //Depending on the pixel selector 
  ImagePixel realInputPixel = m_selector->pickPixel(sel);
  
  //TODO: Search in the logic application to get the input
  //for the algorithm
  
 //SelectionManager *manager = SelectionManager::singleton();
 // pqObjectBuilder* builder = core->getObjectBuilder();
  //pqActiveObjects& activeObjects = pqActiveObjects::instance();
  
}

void SeedGrowingSegmentation::execute()
{
  // Initialize application context
  pqApplicationCore* core = pqApplicationCore::instance();
  pqUndoStack* undoStack = core->getUndoStack();
  pqServerManagerModel* sm = core->getServerManagerModel();
  
  // Pedir los objetos al cachedBuilder
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  
  // Crear una sub-traza con cada paso del algoritmo
  if (cob)
  {
    // make this operation undo-able if undo is enabled
    if (undoStack)
    {
      undoStack->beginUndoSet(QString("Create SeedGrowingSegmentation"));
    }

//pqPipelineSource *input = activeObjects.activeSource();
    pqPipelineSource *filter = cob->get("SeedGrowingSegmentationFilter");
    
    qDebug() << "Threshold: " << m_threshold->value();
    
    //pqPipelineSource *filter = builder->createFilter("filters", "SeedGrowingSegmentationFilter", input);
    //assert(filter);
    //filter->rename("Asymmetric Synapse");
    if (undoStack)
    {
      undoStack->endUndoSet();
    }
    
    // Comment following line to allow several selections 
    emit waitingSelection(NULL);
  }
}



void SeedGrowingSegmentation::abortSelection()
{
  m_addSeed->setChecked(false);
  qDebug() << "Selection aborted";
}


//-----------------------------------------------------------------------------
void SeedGrowingSegmentation::onAction(QAction* action)
{
  if (m_addSeed->isChecked())
    emit waitingSelection(this);
  m_addSeed->setIcon(QIcon(action->icon()));
}

void SeedGrowingSegmentation::setActive(bool active)
{
  if (active)
    emit waitingSelection(this);
  else
    emit waitingSelection(NULL);
}


void SeedGrowingSegmentation::buildUI()
{
  //Threshold
  QLabel *thresholdLabel = new QLabel(tr("Threshold"));
  m_threshold = new QSpinBox();
  
  //Add synapse button
  m_addSeed = new QToolButton();
  m_addSeed->setCheckable(true);
  QMenu *selectors = new QMenu();
  QAction* simplePixel = new QAction(QIcon(":/puntero_mas.svg"), tr("Add synapse (Ctrl +). Exact Pixel"), selectors);
  QAction* maximalPixel = new QAction(QIcon(":/puntero_menos.svg"), tr("Add synapse (Ctrl +). Best Pixel"), selectors);
  selectors->addAction(simplePixel);
  selectors->addAction(maximalPixel);
  m_addSeed->setMenu(selectors);
  m_addSeed->setIcon(QIcon(simplePixel->icon()));

  QHBoxLayout *thresholdLayout = new QHBoxLayout();
  thresholdLayout->addWidget(thresholdLabel);
  thresholdLayout->addWidget(m_threshold);
  thresholdLayout->addWidget(m_addSeed);

  QWidget *thresholdFrame = new QWidget();
  thresholdFrame->setLayout(thresholdLayout);

  QWidgetAction *threshold = new QWidgetAction(this);
  threshold->setDefaultWidget(thresholdFrame);
  
  //Action's Signal connection
  QObject::connect(m_addSeed, SIGNAL(triggered(QAction*)), this, SLOT(onAction(QAction*)));
  QObject::connect(m_addSeed, SIGNAL(toggled(bool)), this, SLOT(setActive(bool)));
}


