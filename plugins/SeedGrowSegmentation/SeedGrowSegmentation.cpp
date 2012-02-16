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

#include "SeedGrowSegmentationFilter.h"

// EspinaModel
#include "SeedGrowSelector.h"
#include "gui/DefaultVOIAction.h"
#include "gui/SegmentAction.h"
#include "gui/ThresholdAction.h"
#include <model/EspinaModel.h>
#include <undo/AddSegmentation.h>
#include <EspinaCore.h>
#include <undo/AddFilter.h>
#include <selection/SelectableItem.h>
#include <undo/AddRelation.h>
#include <model/Channel.h>
#include <model/EspinaFactory.h>

// #include "SeedGrowSegmentationFilter.h"
#include <processing/pqData.h>
#include <selection/PixelSelector.h>
#include <selection/SelectionManager.h>

//GUI includes
#include <QSettings>
#include <QStyle>

#include <pqApplicationCore.h>
#include <pqPipelineFilter.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <vtkSMProxy.h>
#include <vtkSMInputProperty.h>
#include <vtkSMPropertyHelper.h>

#define DEFAULT_THRESHOLD 30


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(QObject* parent)
: QActionGroup(parent)
// , m_defaultVOI(NULL)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_segment       (new SegmentAction(this))
, m_selector      (new SeedGrowSelector(m_threshold))
// , m_preferences(NULL)
{
  // Register Factory's filters
  EspinaFactory::instance()->registerFilter(SGSF, this);

//   m_defaultVOI = new RectangularVOI(false);

  buildSelectors();

  connect(m_segment, SIGNAL(triggered(QAction*)),
	  this, SLOT(waitSeedSelection(QAction*)));
  connect(m_selector.data(), SIGNAL(selectionAborted()),
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

//-----------------------------------------------------------------------------
FilterPtr SeedGrowSegmentation::createFilter(const QString filter, const QString args)
{
  Q_ASSERT(filter == SGSF);

  return FilterPtr(new SeedGrowSegmentationFilter(ModelItem::Arguments(args)));
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::waitSeedSelection(QAction* action)
{
  Q_ASSERT(m_selectors.contains(action));
  m_selector->setPixelSelector(m_selectors[action]);
  m_selector->previewOn();
  SelectionManager::instance()->setSelectionHandler(m_selector.data());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::abortSelection()
{
  m_selector->previewOff();
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

    SelectableItem *input = element.second;

    Q_ASSERT(element.first.size() == 1); // with one pixel
    QVector3D seed = element.first.first();

//     qDebug() << "Channel:" << input->volume().id();
//     qDebug() << "Threshold:" << m_threshold->threshold();
//     qDebug() << "Seed:" << seed;
//     qDebug() << "Use Default VOI:" << m_useDefaultVOI->useDefaultVOI();

    Filter::Arguments args;
    args["Type"] = SGSF;
    args["Channel"]= input->volume().id();
    args["Seed"] = QString("%1,%2,%3").arg(seed.x()).arg(seed.y()).arg(seed.z());
    args["Threshold"] = QString::number(m_threshold->threshold());

    int growSeed[3] = {seed.x(), seed.y(), seed.z()};

    int VOI[6];
    if (m_useDefaultVOI->useDefaultVOI())
    {
      const int W = 60;
      VOI[0] = seed.x() - W;
      VOI[1] = seed.x() + W;
      VOI[2] = seed.y() - W;
      VOI[3] = seed.y() + W;
      VOI[4] = seed.z() - W;
      VOI[5] = seed.z() + W;
    } else
    {
      Q_ASSERT(false);
    }

    QSharedPointer<SeedGrowSegmentationFilter> filter =
      QSharedPointer<SeedGrowSegmentationFilter>(
	new SeedGrowSegmentationFilter(input->volume(),
				     growSeed,
				     m_threshold->threshold(),
				     VOI)
		     );

    Q_ASSERT(filter->numProducts() == 1);
    SegmentationPtr seg(new Segmentation(filter.data(), filter->product(0)));

    TaxonomyNode *tax = EspinaCore::instance()->activeTaxonomy();
    Q_ASSERT(tax);
//     qDebug() << tax->name();

    ModelItem::Vector samples = input->relatedItems(ModelItem::IN, "where");
    Q_ASSERT(samples.size() > 0);
    Sample *sample = dynamic_cast<Sample *>(samples.first());

    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->beginMacro("Add Segmentation");
    undo->push(new AddFilter(filter));
    undo->push(new AddRelation(input,filter.data(),"Channel"));
    undo->push(new AddSegmentation(seg));
    undo->push(new AddRelation(filter, seg, "CreateSegmentation"));
    undo->push(new AddRelation(sample, seg.data(), "where"));
    undo->endMacro();

  // args.insert("VOI",SelectionManager::instance()->voi()->save());
  //createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);

//   IVOI *voi = SelectionManager::instance()->voi();
//   if (!voi && m_useDefaultVOI->isChecked())
//   {
//     voi = m_defaultVOI;
//     Sample *input = EspinaModel::instance()->activeSample();
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

//     m_lastFilter = new SeedGrowSegmentationFilter(args);
//   if (!sgs_sgsf)
//     qWarning() << "SeedGrowSegmentation: Failed to create new segmentation";
  }

//   QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::modifyLastFilter(int value)
{
//   if (m_lastFilter)
//     m_lastFilter->setThreshold(value);
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
