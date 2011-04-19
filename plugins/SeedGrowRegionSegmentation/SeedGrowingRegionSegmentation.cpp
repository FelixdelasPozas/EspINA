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
#include "iPixelSelector.h"
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
SeedGrowingRegionSegmentation::SeedGrowingRegionSegmentation(QObject* parent): EspinaPlugin(), ISegmentationPlugin(parent)
{
  m_selector = new PixelSelector();
  
  buildUI();
  
  connect(this,
	  SIGNAL(waitingSelection(ISelectionHandler *)),
	  SelectionManager::singleton(),
	  SLOT(setSelectionHandler(ISelectionHandler*)));
  connect(this,
	  SIGNAL(productCreated(Segmentation *)),
	  EspINA::instance(),
	  SLOT(addSegmentation(Segmentation*)));
  
  // Init Grow table
  // TODO: Make cleaner
  EspinaArg espina = "input";
  VtkArg vtk = {INPUT,"input"};
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Threshold";
  vtk.type = DOUBLEVECT;
  vtk.name = "Threshold";
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Seed";
  vtk.type = INTVECT;
  vtk.name = "Seed";
  m_tableGrow.addTranslation(espina, vtk);

  // Init Blur table
  espina = "input";
//   vtk = {INPUT,"input"};
  vtk.type = INPUT;
  vtk.name = "input";
  m_tableGrow.addTranslation(espina, vtk);
  espina = "Kernel";
  vtk.type = INTVECT;
  vtk.name = "KernelSize";
  m_tableGrow.addTranslation(espina, vtk);

  m_groupName = "filters";
  m_filterName =  "SeedGrowRegionSegmentationFilter";
  // register in a plugin list
  ProcessingTrace::instance()->registerPlugin(m_groupName, m_filterName, this);

}

void SeedGrowingRegionSegmentation::LoadAnalisys(EspinaParamList& args)
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


void SeedGrowingRegionSegmentation::handle(const Selection sel)
{
  
  qDebug() << "SeedGrowingRegionSegmenation: handling Plugin";
  
  //Depending on the pixel selector 
  ImagePixel realInputPixel = m_selector->pickPixel(sel);
  m_sel.coord = sel.coord;
  m_sel.object = EspINA::instance()->activeSample();
  
  execute();
  
  //TODO: Search in the logic application to get the input
  //for the algorithm
  
 //SelectionManager *manager = SelectionManager::singleton();
  //pqActiveObjects& activeObjects = pqActiveObjects::instance();
  
}

void SeedGrowingRegionSegmentation::execute()
{
  // Initialize application context
  pqApplicationCore* core = pqApplicationCore::instance();
  pqUndoStack* undoStack = core->getUndoStack();
//   pqServerManagerModel* sm = core->getServerManagerModel();
  
  // make this operation undo-able if undo is enabled
  if (undoStack)
  {
    undoStack->beginUndoSet(QString("Create SeedGrowingRegionSegmentation"));
  }
  
  //ProcessingTrace *trace = ProcessingTrace::instance();//!X

  Product *input = dynamic_cast<Product *>(m_sel.object);
  assert (input);


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
  QString seed = QString("%1,%2,%3").arg(m_sel.coord.x).arg(m_sel.coord.y).arg(m_sel.coord.z);
  growArgs.push_back(EspinaParam(QString("Seed"), seed));

  QString th = QString::number(m_threshold->value());
  growArgs.push_back(EspinaParam(QString("Threshold"), th));

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



void SeedGrowingRegionSegmentation::abortSelection()
{
  m_addSeed->setChecked(false);
  qDebug() << "SeedGrowingRegionSegmentation: Selection aborted";
}


//-----------------------------------------------------------------------------
void SeedGrowingRegionSegmentation::onAction(QAction* action)
{
  if (m_addSeed->isChecked())
    emit waitingSelection(this);
  m_addSeed->setIcon(QIcon(action->icon()));
}

void SeedGrowingRegionSegmentation::setActive(bool active)
{
  if (active)
    emit waitingSelection(this);
  else
    emit waitingSelection(NULL);
}


void SeedGrowingRegionSegmentation::buildUI()
{
  //Threshold
  QLabel *thresholdLabel = new QLabel(tr("Region Threshold"));
  m_threshold = new QSpinBox();
  m_threshold->setValue(DEFAULT_THRESHOLD);
  
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
