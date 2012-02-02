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
// #include "SeedGrowSegmentationFilter.h"
#include <paraview/pqData.h>
#include <selection/PixelSelector.h>
#include <selection/SelectionManager.h>

//GUI includes
#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QSpinBox>
#include <QSettings>
#include <QStyle>
#include <QToolButton>
#include <QWidgetAction>

#include <pqApplicationCore.h>
#include <pqPipelineFilter.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <vtkSMProxy.h>
#include <vtkSMInputProperty.h>
#include <vtkSMPropertyHelper.h>


#define DEFAULT_THRESHOLD 30

#define SGS "SeedGrowSegmentation"
#define SGSF "SeedGrowSegmentation::SeedGrowSegmentationFilter"

//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(QObject* parent)
: QActionGroup(parent)
// , m_defaultVOI(NULL)
, m_threshold   (new QSpinBox())
, m_seedSelector(NULL)
, m_eventFilter (new SeedGrowSelector(m_threshold))
// , m_preferences(NULL)
{
//   m_factoryName = SGS;
  // Register Factory's filters
//   ProcessingTrace::instance()->registerPlugin(SGSF, this);
//   EspinaPluginManager::instance()->registerFilter(SGSF,this);
  
//   m_defaultVOI = new RectangularVOI(false);

  buildUI();
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
void SeedGrowSegmentation::changeSeedSelector(QAction *seedSel)
{
  //qDebug() << "SeedGrowSegmenation: Changing Seed Selector";
  m_seedSelector = m_seedSelectors.value(seedSel);

  Q_ASSERT(m_seedSelector != NULL);

  m_segButton->setIcon(seedSel->icon());

  waitSeedSelection(m_segButton->isChecked());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::waitSeedSelection(bool wait)
{
  if (wait)
  {
    if (dynamic_cast<BestPixelSelector*>(m_seedSelector))
      SelectionManager::instance()->setSelectionHandler(m_seedSelector, QCursor(QPixmap(":crossRegion.svg")));
    else
      SelectionManager::instance()->setSelectionHandler(m_seedSelector, Qt::CrossCursor);
    m_segButton->setChecked(true);
  }else
  {
    SelectionManager::instance()->setSelectionHandler(NULL, Qt::ArrowCursor);
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::abortSelection()
{
  QApplication::restoreOverrideCursor();
  m_segButton->setChecked(false);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::startSegmentation(SelectionHandler::MultiSelection sel)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  // Initialize application context
//   pqApplicationCore* core = pqApplicationCore::instance();
  //pqServerManagerModel* sm = core->getServerManagerModel();

  qDebug() << "Start Segmentation";
  Q_ASSERT(sel.size() == 1);// Only one element selected
  SelectionHandler::Selelection element = sel.first();

  pqData input = element.second;

  Q_ASSERT(element.first.size() == 1); // with one pixel
  QVector3D seed = element.first.first();

  qDebug() << "Seed:" << seed;

//   ITraceNode::Arguments args;
//   args.insert("Type", SGSF);
//   args.insert("Seed", QString("%1,%2,%3").arg(seed.x).arg(seed.y).arg(seed.z));
//   args.insert("Threshold",QString::number(m_threshold->value()));
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

//   SeedGrowSegmentationFilter *sgs_sgsf = new SeedGrowSegmentationFilter(input, voi, args);
//   if (!sgs_sgsf)
//     qWarning() << "SeedGrowSegmentation: Failed to create new segmentation";

  QApplication::restoreOverrideCursor();
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
    m_selectors);
  selector = new PixelSelector(m_eventFilter.data());
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  addPixelSelector(action, selector);

  // Best Pixel Selector
  action = new QAction(
    QIcon(":bestPixelSelector.svg")
    , tr("Add synapse (Ctrl +). Best Pixel"),
    m_selectors);
  BestPixelSelector *bestSelector = new BestPixelSelector(m_eventFilter.data());
//   m_preferences = new SeedGrowSegmentationSettings(bestSelector);
//   EspinaPluginManager::instance()->registerPreferencePanel(m_preferences);
//   QSettings settings;
//   if (settings.contains(BEST_PIXEL))
//     bestSelector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  addPixelSelector(action, selector);

  m_seedSelector = selector;
  m_segButton->setIcon(action->icon());
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::buildUI()
{
  // Threshold Widget
  QLabel *thresholdLabel = new QLabel(tr("Threshold"));
  m_threshold->setMinimum(0);
  m_threshold->setMaximum(255);
  m_threshold->setValue(DEFAULT_THRESHOLD);
  m_threshold->setToolTip(tr("Determine the size of color value range for a given pixel"));

  // Use default VOI
  m_useDefaultVOI = new QCheckBox(tr("Default VOI"));
  m_useDefaultVOI->setCheckState(Qt::Checked);

  // Segmentation Button
  m_segButton = new QToolButton();
  m_segButton->setCheckable(true);
  m_selectors = new QMenu();
  m_segButton->setAutoRaise(true);
  m_segButton->setIconSize(QSize(22,22));

  buildSelectors();

  m_segButton->setMenu(m_selectors);
  m_segButton->setToolTip(tr("Pixel selector"));

  // Plugin's Widget Layout
  QHBoxLayout *thresholdLayout = new QHBoxLayout();
  thresholdLayout->addWidget(thresholdLabel);
  thresholdLayout->addWidget(m_threshold);
  thresholdLayout->addWidget(m_useDefaultVOI);
  thresholdLayout->addWidget(m_segButton);

  QWidget *thresholdFrame = new QWidget();
  thresholdFrame->setLayout(thresholdLayout);

  QWidgetAction *threshold = new QWidgetAction(this);
  threshold->setDefaultWidget(thresholdFrame);

  // Interface connections
  QObject::connect(m_segButton, SIGNAL(triggered(QAction*)), this, SLOT(changeSeedSelector(QAction *)));
  QObject::connect(m_segButton, SIGNAL(toggled(bool)), this, SLOT(waitSeedSelection(bool)));
  //QObject::connect(m_segButton, SIGNAL(clicked(bool)), this, SLOT(setActive(bool)));
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::addPixelSelector(QAction* action, SelectionHandler* handler)
{
  m_selectors->addAction(action);
  connect(handler, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(startSegmentation(SelectionHandler::MultiSelection)));
  connect(handler, SIGNAL(selectionAborted()),
	  this, SLOT(abortSelection()));
  m_seedSelectors.insert(action, handler);
}