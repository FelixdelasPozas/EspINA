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
#include "SeedGrowSegmentation.h"

#include <QDebug>

// EspINA
#include "SeedGrowSelector.h"
#include "gui/DefaultVOIAction.h"
#include "gui/SegmentAction.h"
#include "gui/ThresholdAction.h"

// #include "SeedGrowSegmentationFilter.h"
#include <processing/pqData.h>
#include <selection/PixelSelector.h>
#include <selection/SelectionManager.h>

//GUI includes
#include <QApplication>
#include <QSettings>
#include <QStyle>

#include <pqApplicationCore.h>
#include <pqPipelineFilter.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <vtkSMProxy.h>
#include <vtkSMInputProperty.h>
#include <vtkSMPropertyHelper.h>
#include "SeedGrowSegmentationFilter.h"

#define DEFAULT_THRESHOLD 30

#include <QDebug>

#define SGS "SeedGrowSegmentation"
#define SGSF "SeedGrowSegmentation::SeedGrowSegmentationFilter"

//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(QObject* parent)
: QActionGroup(parent)
// , m_defaultVOI(NULL)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_segment       (new SegmentAction(this))
, m_eventFilter   (new SeedGrowSelector(m_threshold))
, m_lastFilter    (NULL)
// , m_preferences(NULL)
{
//   m_factoryName = SGS;
  // Register Factory's filters
//   ProcessingTrace::instance()->registerPlugin(SGSF, this);
//   EspinaPluginManager::instance()->registerFilter(SGSF,this);

//   m_defaultVOI = new RectangularVOI(false);

  buildSelectors();

  connect(m_segment, SIGNAL(triggered(QAction*)),
	  this, SLOT(waitSeedSelection(QAction*)));
  connect(m_eventFilter.data(), SIGNAL(selectionAborted()),
	  this, SLOT(onSelectionAborted()));
  connect(m_segment, SIGNAL(actionCanceled()),
	  this, SLOT(abortSelection()));
  connect(m_threshold,SIGNAL(thresholdChanged(int)),
	  this, SLOT(modifyLastFilter(int)));
}


//-----------------------------------------------------------------------------
SeedGrowSegmentation::~SeedGrowSegmentation()
{
}

// //-----------------------------------------------------------------------------
// EspinaFilter *SeedGrowSegmentation::createFilter(QString filter, ITraceNode::Arguments & args)
// {
//   if (filter == SGSF)
//   {
//     SeedGrowSegmentationFilter *sgs_sgsf = new SeedGrowSegmentationFilter(args);
//     return sgs_sgsf;
//   }
//   qWarning("SeedGrowSegmenation::createFilter: Error no such a Filter");
//   return NULL;
// }


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::waitSeedSelection(QAction* action)
{
  Q_ASSERT(m_selectors.contains(action));
  m_eventFilter->setPixelSelector(m_selectors[action]);
  SelectionManager::instance()->setSelectionHandler(m_eventFilter.data());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::abortSelection()
{
  SelectionManager::instance()->setSelectionHandler(NULL);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::onSelectionAborted()
{
  m_segment->cancel();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::startSegmentation(SelectionHandler::MultiSelection sel)
{
//   QApplication::setOverrideCursor(Qt::WaitCursor);

  // Initialize application context
//   pqApplicationCore* core = pqApplicationCore::instance();
  //pqServerManagerModel* sm = core->getServerManagerModel();

  if (sel.size() > 0)
  {
    qDebug() << "Start Segmentation";
    Q_ASSERT(sel.size() == 1);// Only one element selected
    SelectionHandler::Selelection element = sel.first();

    pqData input = element.second;

    Q_ASSERT(element.first.size() == 1); // with one pixel
    QVector3D seed = element.first.first();

    qDebug() << "Channel:" << input.id();
    qDebug() << "Threshold:" << m_threshold->threshold();
    qDebug() << "Seed:" << seed;
    qDebug() << "Use Default VOI:" << m_useDefaultVOI->useDefaultVOI();

    Filter::Arguments args;
    args["Type"] = SGSF;
    args["Channel"]= input.id();
    args["Seed"] = QString("%1,%2,%3").arg(seed.x()).arg(seed.y()).arg(seed.z());
    args["Threshold"] = QString::number(m_threshold->threshold());
  // args.insert("VOI",SelectionManager::instance()->voi()->save());
  //createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);

//   IVOI *voi = SelectionManager::instance()->voi();
//   if (!voi && m_useDefaultVOI->isChecked())
//   {
//     voi = m_defaultVOI;
//     Sample *input = EspINA::instance()->activeSample();
//     voi->setSource(input);
//     double spacing[3];
//     input->spacing(spacing);
//     double defVOI[6] = {(seed.x - m_preferences->xSize())*spacing[0],
// 			(seed.x + m_preferences->xSize())*spacing[0],
// 			(seed.y - m_preferences->ySize())*spacing[1],
// 			(seed.y + m_preferences->ySize())*spacing[1],
// 			(seed.z - m_preferences->zSize())*spacing[2],
// 			(seed.z + m_preferences->zSize())*spacing[2]};
//     vtkSMPropertyHelper(voi->getProxy(),"Bounds").Set(defVOI,6);
//     voi->getProxy()->UpdateVTKObjects();
// //     double checkBounds[6];
//     vtkSMPropertyHelper(voi->getProxy(),"Bounds").Get(defVOI,6);
//   }

    m_lastFilter = new SeedGrowSegmentationFilter(args);
//   if (!sgs_sgsf)
//     qWarning() << "SeedGrowSegmentation: Failed to create new segmentation";
  }

//   QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::modifyLastFilter(int value)
{
  if (m_lastFilter)
    m_lastFilter->setThreshold(value);
}


//------------------------------------------------------------------------
void SeedGrowSegmentation::addPixelSelector(QAction* action, SelectionHandler* handler)
{
  m_segment->addSelector(action);
  m_selectors[action] = handler;
  connect(handler, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(startSegmentation(SelectionHandler::MultiSelection)));
//   connect(handler, SIGNAL(selectionAborted()),
// 	  this, SLOT(abortSelection()));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::buildSelectors()
{
  SelectionHandler *selector;
  QAction *action;

  // Exact Pixel Selector
  action = new QAction(
    QIcon(":pixelSelector.svg")
    , tr("Add synapse (Ctrl +). Exact Pixel"),
    m_segment);
  selector = new PixelSelector();
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  addPixelSelector(action, selector);

//   // Best Pixel Selector
  action = new QAction(
    QIcon(":bestPixelSelector.svg")
    , tr("Add synapse (Ctrl +). Best Pixel"),
    m_segment);
  BestPixelSelector *bestSelector = new BestPixelSelector();
//   m_preferences = new SeedGrowSegmentationSettings(bestSelector);
//   EspinaPluginManager::instance()->registerPreferencePanel(m_preferences);
//   QSettings settings;
//   if (settings.contains(BEST_PIXEL))
//     bestSelector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  selector->setCursor(QCursor(QPixmap(":crossRegion.svg")));
  addPixelSelector(action, selector);
}
