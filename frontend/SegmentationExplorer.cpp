/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//TODO: ITS ONLY A PROTOTYPE

#include "SegmentationExplorer.h"
#include <pqObjectBuilder.h>
#include <pqApplicationCore.h>
#include <pqActiveObjects.h>
#include <pqRenderView.h>
#include <espina.h>
#include "traceNodes.h"
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>


SegmentationExplorer::SegmentationExplorer(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, view(NULL)
{
  setupUi(this);
}

void SegmentationExplorer::setVisible(bool visible)
{
  if (!view)
  {
    pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
    pqServer * server= pqActiveObjects::instance().activeServer();
    view = qobject_cast<pqRenderView*>(builder->createView( pqRenderView::renderViewType(), server));
    this->viewLayout->addWidget(view->getWidget());
    view->setCenterAxesVisibility(false);
    
    pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
    pqDataRepresentation *dr = dp->setRepresentationVisibility(
      EspINA::instance()->segmentations(EspINA::instance()->activeSample()).first()->outputPort()
      ,view,true);
    pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
    assert(rep);
    rep->setRepresentation(4);
  }
  QWidget::setVisible(visible);
}
