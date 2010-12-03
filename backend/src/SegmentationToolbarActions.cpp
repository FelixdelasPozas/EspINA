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
#include "SegmentationToolbarActions.h"

#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QWidgetAction>

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqActiveObjects.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "pqPipelineFilter.h"
#include "vtkSMProxy.h"
#include "vtkSMInputProperty.h"

#include <QDebug>

//#include "../../frontend/src/sliceWidget.h"

//extern SliceWidget *vista;

//-----------------------------------------------------------------------------
SegmentationToolbarActions::SegmentationToolbarActions(QObject* p) : QActionGroup(p)
{
  //Threshold
  QWidgetAction *threshold = new QWidgetAction(this);
  QWidget *thresholdFrame = new QWidget();
  QHBoxLayout *thresholdLayout = new QHBoxLayout();
  QLabel *thresholdLabel = new QLabel(tr("Threshold"));
  m_threshold = new QSpinBox();
  thresholdLayout->addWidget(thresholdLabel);
  thresholdLayout->addWidget(m_threshold);
  thresholdFrame->setLayout(thresholdLayout);
  threshold->setDefaultWidget(thresholdFrame);
  //Add synapse
  QAction* add = new QAction(QIcon(":/puntero_mas.svg"), tr("Add synapse (Ctrl +)"), this);
  add->setData("AddSynapse");
  this->addAction(add);
  //Remove synapse
  QAction *remove = new QAction(QIcon(":/puntero_menos.svg"), tr("Remove synapse (Ctrl -)"), this);
  remove->setData("RemoveSynapse");
  this->addAction(remove);
  //Action's Signal connection
  QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onAction(QAction*)));
}

//-----------------------------------------------------------------------------
void SegmentationToolbarActions::onAction(QAction* a)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqServerManagerModel* sm = core->getServerManagerModel();
  pqUndoStack* stack = core->getUndoStack();

  /// Check that we are connect to some server (either builtin or remote).
  if (sm->getNumberOfItems<pqServer*>())
    {
    // just create it on the first server connection
    pqServer* s = sm->getItemAtIndex<pqServer*>(0);
    QString source_type = a->data().toString();
    // make this operation undo-able if undo is enabled
    if (stack)
      {
      stack->beginUndoSet(QString("Create %1").arg(source_type));
      }
	if (source_type == tr("AddSynapse"))
	{
		pqPipelineSource *currentStack = activeObjects.activeSource();
		qDebug() << "Threshold: " << m_threshold->value();
		pqPipelineSource *filter = builder->createFilter("filters", "SegmentationFilter", currentStack,0);
		filter->rename("Asymmetric Synapse");
	}
	//vista->showSource(filter->getOutputPort(0),true);
    if (stack)
      {
      stack->endUndoSet();
      }
    }
}

