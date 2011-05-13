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

#include "RectangularVOI.h"

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
, m_activeVOI(NULL)
{
  m_pluginName = "VolumeOfInterest";
  
  buildUI();
  
  // register in a plugin list
  QString registerName = m_pluginName + "::" + "RectangularVOIFilter::Apply";
  ProcessingTrace::instance()->registerPlugin(registerName, this);
  registerName = m_pluginName + "::" + "RectangularVOIFilter::Restore";
  ProcessingTrace::instance()->registerPlugin(registerName, this);
}

//-----------------------------------------------------------------------------
IFilter* VolumeOfInterest::createFilter(QString filter, ITraceNode::Arguments& args)
{
  QStringList filterNames = filter.split("::");
  if (filterNames[1] == "RectangularVOIFilter")
  {
    //m_VOIs[filterNames[1]
    //! delegar en la estrategia que corresponda (m_VOIs)
    //return m_VOIs[X]->createApplyFilter();
//     CachedObjectBuilder *cob = CachedObjectBuilder::instance();
//     vtkFilter::Arguments filterArgs;
//     filterArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, args["input"]));
//     filterArgs.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, args["VOI"]));
//     vtkFilter *applyFilter = cob->createFilter("filters","VOI",filterArgs);
  }else
  { 
    qDebug("SeedGrowSegmenation::LoadAnalisys: Error no such a Filter");
  }
}


//-----------------------------------------------------------------------------
void VolumeOfInterest::enable(bool value)
{
  if (m_voiButton->isChecked())
  {
    qDebug() << "EspINA::VolumeOfInterest: Apply VOI";
    SelectionManager::instance()->setVOI(m_activeVOI);
    m_voiButton->setChecked(true);
  }else
  {
    SelectionManager::instance()->setVOI(NULL);
  }
}


//-----------------------------------------------------------------------------
void VolumeOfInterest::changeVOI(QAction* voi)
{
  qDebug() << "EspINA::VolumeOfInterest: Changing VOI";
  m_activeVOI = m_VOIs.value(voi);
  
  if (!m_activeVOI)
  {
    qDebug() << "EspINA::VolumeOfInterest FATAL ERROR: No valid VOI";
    assert(m_activeVOI);
  }
  
  m_voiButton->setIcon(voi->icon());
  
  enable(m_voiButton->isChecked());
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::cancelVOI()
{
  m_voiButton->setChecked(false);
}


//-----------------------------------------------------------------------------
void VolumeOfInterest::buildVOIs()
{
  IVOI *voi;
  QAction *action;
  
  // Exact Pixel Selector
  action = new QAction(
    QIcon(":/voi")
    , tr("Volume Of Interest"),
    m_VOIMenu);
  voi = new RectangularVOI();
  addVOI(action, voi);
}

void VolumeOfInterest::buildUI()
{
  // VOI Button
  m_voiButton = new QToolButton();
  m_voiButton->setCheckable(true);
  m_VOIMenu = new QMenu();
  
  buildVOIs();
  
  m_activeVOI = m_VOIs.value(m_VOIs.keys().first());
  m_voiButton->setIcon(m_VOIs.key(m_activeVOI)->icon());
  m_voiButton->setMenu(m_VOIMenu);
  
  QWidgetAction *threshold = new QWidgetAction(this);
  threshold->setDefaultWidget(m_voiButton);
  
  // Interface connections
  connect(m_voiButton, SIGNAL(triggered(QAction*)), this, SLOT(changeVOI(QAction*)));
  connect(m_voiButton, SIGNAL(toggled(bool)), this, SLOT(enable(bool)));
}


//------------------------------------------------------------------------
void VolumeOfInterest::addVOI(QAction* action, IVOI* voi)
{
  m_VOIMenu->addAction(action);
  /*
  connect(voi,
	  SIGNAL(selectionChanged(ISelectionHandler::Selection)),
	  this,
	  SLOT(startSegmentation(ISelectionHandler::Selection)));
  connect(voi,
	  SIGNAL(selectionAborted()),
	  this,
	  SLOT(abortSelection()));
	  */
  m_VOIs.insert(action, voi);
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
