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
#include "SeedGrowingRegionSegmentation.h"
#include "selectionManager.h"
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
SeedGrowingRegionSegmentation::SeedGrowingRegionSegmentation(QObject* parent)
: ISegmentationPlugin(parent)
, EspinaPlugin()
, m_seedSelector(NULL)
{
  m_groupName = "filters";
  m_filterName = "SeedGrowRegionSegmentationFilter";
  
  initBlurTable();
  initGrowTable();
  
  buildUI();
  
  connect(this,
	  SIGNAL(productCreated(Segmentation *)),
	  EspINA::instance(),
	  SLOT(addSegmentation(Segmentation*)));
  
  // register in a plugin list
  ProcessingTrace::instance()->registerPlugin(this);

}

void SeedGrowingRegionSegmentation::LoadAnalisys(QString& filter, EspinaParamList& args)
{
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
    qDebug("SeedGrowingRegionSegmenation::LoadAnalisys: Error loading a tarce file. \"input\" argument not found");
    exit(-1);//throw Finalizar importacion
  }
  //Product* input = dynamic_cast<Product*> (Cache::instance()->getEntry(InputId));
  
  this->buildSubPipeline(EspINA::instance()->activeSample(), args);
}

//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::changeSeedSelector(QAction* seedSel)
{
  qDebug() << "EspINA::SeedGrowingRegionSegmenation: Changing Seed Selector";
  m_seedSelector = m_seedSelectors.value(seedSel);
  
  if (!m_seedSelector)
  {
    qDebug() << "EspINA::SeedGrowingRegionSegmentation FATAL ERROR: No valid Seed Selector";
    assert(m_seedSelector);
  }
  
  m_segButton->setIcon(seedSel->icon());
  
  waitSeedSelection(m_segButton->isChecked());
}

//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::waitSeedSelection(bool wait)
{
  if (wait)
  {
    qDebug() << "EspINA::SeedGrowingRegionSegmenation: Waiting for Seed Selection";
    SelectionManager::instance()->setSelectionHandler(m_seedSelector);
    m_segButton->setChecked(true);
  }else
  {
    SelectionManager::instance()->setSelectionHandler(NULL);
  }
}

//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::abortSelection()
{
  m_segButton->setChecked(false);
}

//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::startSegmentation(ISelectionHandler::Selection sel)
{
  qDebug() << "EspINA::SeedGrowRegionSegmenation: Start Segmentation";
  
  // Initialize application context
  pqApplicationCore* core = pqApplicationCore::instance();
  pqUndoStack* undoStack = core->getUndoStack();
  //pqServerManagerModel* sm = core->getServerManagerModel();
  
  // make this operation undo-able if undo is enabled
  if (undoStack)
  {
    undoStack->beginUndoSet(QString("Create SeedGrowingRegionSegmentation"));
  }
  
  assert(sel.size() == 1);// Only one element selected
  ISelectionHandler::SelElement element = sel.first();
  
  Product *input = element.second;
  assert (input);
  
  assert(element.first.size() == 1); // with one pixel
  Point seed = element.first.first();

  // Crear los Filtros
  /*
  EspinaParamList blurArgs;
  blurArgs.push_back(EspinaParam("input",input->id()));
  QString kernel = QString("2,2,2");
  blurArgs.push_back(EspinaParam("Kernel",kernel.toStdString()));

  Filter *blur = new Filter("filter","Median",blurArgs,m_tableBlur); 
  
  assert(blur->products().size() == 1);
  */

  EspinaParamList growArgs;
  growArgs.push_back(EspinaParam(QString("input"), input->id()));
  QString seedArg = QString("%1,%2,%3").arg(seed.x).arg(seed.y).arg(seed.z);
  growArgs.push_back(EspinaParam(QString("Seed"), seedArg));

  QString thArg = QString::number(m_threshold->value());
  growArgs.push_back(EspinaParam(QString("Threshold"), thArg));

  this->buildSubPipeline(input, growArgs);

  if (undoStack)
  {
    undoStack->endUndoSet();
  }
  
//   ProcessingTrace* p = ProcessingTrace::instance();
//   qDebug("TRACE PRINT 1");
//   p->print(std::cout);
//   fstream f1 ("/tmp/traza.dot", fstream::in | fstream::out | fstream::trunc );
//   p->print(f1);
//   
//   f1.seekg(0);
//   p->readTrace(f1);
//   qDebug("TRACE PRINT 2");
//   p->print(std::cout);
  // Comment following line to allow several selections 
  //emit waitingSelection(NULL);
}



//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::buildSelectors()
{
  ISelectionHandler *handler;
  QAction *action;
  
  // Exact Pixel Selector
  action = new QAction(
    QIcon(":/pixelSel")
    , tr("Add synapse (Ctrl +). Exact Pixel"),
    m_selectors);
  handler = new PixelSelector();
  handler->multiSelection = false;
  handler->filters << "EspINA_Sample";
  addPixelSelector(action, handler);
  
  // Best Pixel Selector
  action = new QAction(
    QIcon(":/bestPixelSel")
    , tr("Add synapse (Ctrl +). Best Pixel"),
    m_selectors);
  handler = new BestPixelSelector();
  handler->multiSelection = false;
  handler->filters << "EspINA_Sample";
  addPixelSelector(action, handler);
}

void SeedGrowingRegionSegmentation::buildUI()
{
  //Threshold Widget
  QLabel *thresholdLabel = new QLabel(tr("Region Threshold"));
  m_threshold = new QSpinBox();
  m_threshold->setValue(DEFAULT_THRESHOLD);
  
  //Segmentation Button
  m_segButton = new QToolButton();
  m_segButton->setCheckable(true);
  m_selectors = new QMenu();
  
  buildSelectors();
  
  m_seedSelector = m_seedSelectors.value(m_seedSelectors.keys().first());
  m_segButton->setIcon(m_seedSelectors.key(m_seedSelector)->icon());
  m_segButton->setMenu(m_selectors);
  
  // Plugin's Widget Layout
  QHBoxLayout *thresholdLayout = new QHBoxLayout();
  thresholdLayout->addWidget(thresholdLabel);
  thresholdLayout->addWidget(m_threshold);
  thresholdLayout->addWidget(m_segButton);
  
  QWidget *thresholdFrame = new QWidget();
  thresholdFrame->setLayout(thresholdLayout);

  QWidgetAction *threshold = new QWidgetAction(this);
  threshold->setDefaultWidget(thresholdFrame);
  
  // Interface connections
  QObject::connect(m_segButton, SIGNAL(triggered(QAction*)), this, SLOT(changeSeedSelector(QAction *)));
  QObject::connect(m_segButton, SIGNAL(toggled(bool)), this, SLOT(waitSeedSelection(bool)));
}


//------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::initBlurTable()
{
  EspinaArg espina = "input";
  VtkArg vtk;
  espina = "input";
  vtk.type = INPUT;
  vtk.name = "input";
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Kernel";
  vtk.type = INTVECT;
  vtk.name = "KernelSize";
  m_tableGrow.addTranslation(espina, vtk);
}

//------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::initGrowTable()
{
  EspinaArg espina = "input";
  VtkArg vtk;
  vtk.type = INPUT;
  vtk.name = "input";
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Threshold";
  vtk.type = DOUBLEVECT;
  vtk.name = "Threshold";
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Seed";
  vtk.type = INTVECT;
  vtk.name = "Seed";
  m_tableGrow.addTranslation(espina, vtk);
}

//------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::addPixelSelector(QAction* action, ISelectionHandler* handler)
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
void SeedGrowingRegionSegmentation::buildSubPipeline(Product* input, EspinaParamList args)
{
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
}
