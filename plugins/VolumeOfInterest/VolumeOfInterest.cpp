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
#include "VolumeOfInterest.h"

//#include "objectManager.h"
#include "espina.h"
#include <cache/cachedObjectBuilder.h>

#include "pixelSelector.h"
#include "filter.h"

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
#include <espINAFactory.h>


#define DEFAULT_THRESHOLD 30

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(QObject* parent)
: QActionGroup(parent)
, EspinaPlugin()
, m_seedSelector(NULL)
{
  m_groupName = "filters";
  m_filterName = "VolumeOfInterest"; //TODO: Review this
  
  buildUI();
  
  // register in a plugin list
  ProcessingTrace::instance()->registerPlugin(this);

}

void VolumeOfInterest::LoadAnalisys(EspinaParamList& args)
{
  assert(false);
  QString InputId = "";
  EspinaParamList::iterator it;
  for(it=args.begin(); it != args.end(); it++)
  {
    if( (*it).first == "input" ){
      InputId = (*it).second;
      break;
    }
  }
  
  if( InputId.isEmpty() ){
    qDebug("VolumeOfInterest::LoadAnalisys: Error loading a tarce file. \"input\" argument not found");
    exit(-1);//throw Finalizar importacion
  }
  //Product* input = dynamic_cast<Product*> (Cache::instance()->getEntry(InputId));
  
  this->buildSubPipeline(EspINA::instance()->activeSample(), args);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::changeSeedSelector(QAction* seedSel)
{
  assert(false);
  qDebug() << "EspINA::SeedGrowingRegionSegmenation: Changing Seed Selector";
  m_seedSelector = m_seedSelectors.value(seedSel);
  
  if (!m_seedSelector)
  {
    qDebug() << "EspINA::VolumeOfInterest FATAL ERROR: No valid Seed Selector";
    assert(m_seedSelector);
  }
  
  m_roiButton->setIcon(seedSel->icon());
  
  waitSeedSelection(m_roiButton->isChecked());
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::waitSeedSelection(bool wait)
{
  assert(false);
  if (wait)
  {
    qDebug() << "EspINA::SeedGrowingRegionSegmenation: Waiting for Seed Selection";
    SelectionManager::instance()->setSelectionHandler(m_seedSelector);
    m_roiButton->setChecked(true);
  }else
  {
    SelectionManager::instance()->setSelectionHandler(NULL);
  }
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::abortSelection()
{
  m_roiButton->setChecked(false);
}


//-----------------------------------------------------------------------------
void VolumeOfInterest::buildSelectors()
{
  ISelectionHandler *handler;
  QAction *action;
  
  // Exact Pixel Selector
  action = new QAction(
    QIcon(":/voi")
    , tr("Volume Of Interest"),
    m_selectors);
  handler = new PixelSelector();
  handler->multiSelection = false;
  handler->filters << "EspINA_Sample";
  addPixelSelector(action, handler);
  
  // Best Pixel Selector
  action = new QAction(
    QIcon(":/voi")
    , tr("Add synapse (Ctrl +). Best Pixel"),
    m_selectors);
  handler = new BestPixelSelector();
  handler->multiSelection = false;
  handler->filters << "EspINA_Sample";
  addPixelSelector(action, handler);
}

void VolumeOfInterest::buildUI()
{
  // VOI Button
  m_roiButton = new QToolButton();
  m_roiButton->setCheckable(true);
  m_selectors = new QMenu();
  
  buildSelectors();
  
  m_seedSelector = m_seedSelectors.value(m_seedSelectors.keys().first());
  m_roiButton->setIcon(m_seedSelectors.key(m_seedSelector)->icon());
  m_roiButton->setMenu(m_selectors);
  
  QWidgetAction *threshold = new QWidgetAction(this);
  threshold->setDefaultWidget(m_roiButton);
  
  // Interface connections
  QObject::connect(m_roiButton, SIGNAL(triggered(QAction*)), this, SLOT(changeSeedSelector(QAction *)));
  QObject::connect(m_roiButton, SIGNAL(toggled(bool)), this, SLOT(waitSeedSelection(bool)));
}


//------------------------------------------------------------------------
void VolumeOfInterest::addPixelSelector(QAction* action, ISelectionHandler* handler)
{
  m_selectors->addAction(action);
  connect(handler,
	  SIGNAL(selectionChanged(ISelectionHandler::Selection)),
	  this,
	  SLOT(startSegmentation(ISelectionHandler::Selection)));
  connect(handler,
	  SIGNAL(selectionAborted()),
	  this,
	  SLOT(abortSelection()));
  m_seedSelectors.insert(action, handler);
}


//------------------------------------------------------------------------
//! Creates the corresponding Pipeline of the plugin (the Filters and the Products). It also updates the Trace of the system
void VolumeOfInterest::buildSubPipeline(Product* input, EspinaParamList args)
{
  /*
  ProcessingTrace *trace = ProcessingTrace::instance();//!X

  Filter *grow = new Filter(
    m_groupName,
    m_filterName,
    args,
    m_tableGrow
  );
  
  trace->connect(input, grow, "input");
   
  Product *product;
  foreach(product,grow->products())
  {
    Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(product->sourceData(),product->portNumber(), grow->id());
    emit productCreated(seg);
  }
  */
}
