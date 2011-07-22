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

// EspINA
#include "espina.h"
#include "segmentation.h"

#include <pqObjectBuilder.h>
#include <pqApplicationCore.h>
#include <pqActiveObjects.h>
#include <pqRenderView.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
//#include "../plugins/SeedGrowSegmentation/SeedGrowSegmentationFilter.h"


SegmentationExplorer::SegmentationExplorer(Segmentation *seg, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, view(NULL)
{
  setupUi(this);
  
  if (!view)
  {
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    pqServer * server= pqActiveObjects::instance().activeServer();
    view = qobject_cast<pqRenderView*>(ob->createView( pqRenderView::renderViewType(), server));
    
    this->viewLayout->addWidget(view->getWidget());
    view->setCenterAxesVisibility(false);
    
    seg->representation("Mesh")->render(view);
    //SeedGrowSegmentationFilter *filter = dynamic_cast<SeedGrowSegmentationFilter*>(seg->parent());
    //this->m_threshold->setValue(filter->threshold());
  }
}

SegmentationExplorer::~SegmentationExplorer()
{
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  ob->destroy(view);
}

