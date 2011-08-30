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
#include "crosshairExtension.h"

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
#include <sample.h>



#define DEFAULT_THRESHOLD 30

//-----------------------------------------------------------------------------
VolumeOfInterest::VolumeOfInterest(QObject* parent)
: QActionGroup(parent)
, m_activeVOI(NULL)
, m_focusedSample(NULL)
{
  buildUI();
  
  // register in a plugin list
  //QString registerName = m_pluginName + "::" + "RectangularVOIFilter::Apply";
  //ProcessingTrace::instance()->registerPlugin(registerName, this);
  //registerName = m_pluginName + "::" + "RectangularVOIFilter::Restore";
  //ProcessingTrace::instance()->registerPlugin(registerName, this);
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
void VolumeOfInterest::focusSampleChanged(Sample* sample)
{
  if (sample)
  {
    m_voiButton->setEnabled(true);
    // Update limits
    int mextent[6];
    sample->extent(mextent);
    int minSlices = mextent[4] + SliceOffset;
    int maxSlices = mextent[5] + SliceOffset;
    m_fromSlice->setMinimum(minSlices);
    m_fromSlice->setMaximum(maxSlices);
    m_toSlice->setMinimum(minSlices);
    m_toSlice->setMaximum(maxSlices);
    m_toSlice->setValue(maxSlices);
  }
  else
  {
    m_voiButton->setEnabled(false);
  }
  m_focusedSample = sample;
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setFromCurrentSlice()
{
  if (m_focusedSample)
  {
    CrosshairExtension::SampleRepresentation *rep =
      dynamic_cast<CrosshairExtension::SampleRepresentation *>(
	m_focusedSample->representation(CrosshairExtension::SampleRepresentation::ID)
	);
    assert(rep);
    m_fromSlice->setValue(rep->slice(VIEW_PLANE_XY)+SliceOffset);
  }
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setToCurrentSlice()
{
  if (m_focusedSample)
  {
    CrosshairExtension::SampleRepresentation *rep =
      dynamic_cast<CrosshairExtension::SampleRepresentation *>(
	m_focusedSample->representation(CrosshairExtension::SampleRepresentation::ID)
	);
    assert(rep);
    m_toSlice->setValue(rep->slice(VIEW_PLANE_XY)+SliceOffset);
  }
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setFromSlice(int value)
{
  if (m_activeVOI && m_voiButton->isChecked())
    m_activeVOI->setFromSlice(value-SliceOffset);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::setToSlice(int value)
{
  if (m_activeVOI && m_voiButton->isChecked())
    m_activeVOI->setToSlice(value-SliceOffset);
}

//-----------------------------------------------------------------------------
void VolumeOfInterest::buildVOIs()
{
  IVOI *voi;
  QAction *action;
  
  // Exact Pixel Selector
  action = new QAction(
    QIcon(":roi.svg")
    , tr("Volume Of Interest"),
    m_VOIMenu);
  voi = new RectangularVOI();
  addVOI(action, voi);
  connect(voi, SIGNAL(voiCancelled()),this,SLOT(cancelVOI()));
}

void VolumeOfInterest::buildUI()
{
  // VOI Button
  m_voiButton = new QToolButton();
  m_voiButton->setCheckable(true);
  m_VOIMenu = new QMenu();
  m_voiButton->setAutoRaise(true);
  m_voiButton->setIconSize(QSize(22,22));
  m_voiButton->setEnabled(false);
  
  buildVOIs();
  
  m_activeVOI = m_VOIs.value(m_VOIs.keys().first());
  m_voiButton->setIcon(m_VOIs.key(m_activeVOI)->icon());
  m_voiButton->setMenu(m_VOIMenu);
  
  QToolButton *updateFrom = new QToolButton();
  updateFrom->setText(tr("From"));
  updateFrom->setAutoRaise(true);
  connect(updateFrom,SIGNAL(clicked(bool)),this,SLOT(setFromCurrentSlice()));

  m_fromSlice = new QSpinBox();
  m_fromSlice->setMinimum(0);
  m_fromSlice->setMaximum(0);
  m_fromSlice->setToolTip(tr("Determine which is the first slice included in the VOI"));
  connect(m_fromSlice,SIGNAL(valueChanged(int)),this,SLOT(setFromSlice(int)));
  
  QToolButton *updateTo = new QToolButton();
  updateTo->setText(tr("To"));
  updateTo->setAutoRaise(true);
  connect(updateTo,SIGNAL(clicked(bool)),this,SLOT(setToCurrentSlice()));

  m_toSlice = new QSpinBox();
  m_toSlice->setMinimum(0);
  m_toSlice->setMaximum(0);
  m_toSlice->setToolTip(tr("Determine which is the last slice included in the VOI"));
  connect(m_toSlice,SIGNAL(valueChanged(int)),this,SLOT(setToSlice(int)));
  
  // Plugin's Widget Layout
  QHBoxLayout *toolbarLayout = new QHBoxLayout();
  toolbarLayout->addWidget(updateFrom);
  toolbarLayout->addWidget(m_fromSlice);
  toolbarLayout->addWidget(updateTo);
  toolbarLayout->addWidget(m_toSlice);
  toolbarLayout->addWidget(m_voiButton);
    
  QWidget *toolbar = new QWidget();
  toolbar->setLayout(toolbarLayout);

  QWidgetAction *toolbarAdaptor = new QWidgetAction(this);
  toolbarAdaptor->setDefaultWidget(toolbar);
  
  // Interface connections
  connect(m_voiButton, SIGNAL(triggered(QAction*)), this, SLOT(changeVOI(QAction*)));
  connect(m_voiButton, SIGNAL(toggled(bool)), this, SLOT(enable(bool)));
  connect(EspINA::instance(), SIGNAL(focusSampleChanged(Sample*)),
          this, SLOT(focusSampleChanged(Sample *)));
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
