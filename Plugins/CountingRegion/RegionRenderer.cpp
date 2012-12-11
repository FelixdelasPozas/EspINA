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
#include "RegionRenderer.h"

#include "CountingRegion.h"
#include "regions/BoundingRegion.h"

#include <Core/Model/ModelItem.h>

#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

//-----------------------------------------------------------------------------
RegionRenderer::RegionRenderer(CountingRegion* plugin)
: m_plugin(plugin)
{
  connect(m_plugin, SIGNAL(regionCreated(BoundingRegion*)),
          this, SLOT(regionCreated(BoundingRegion*)));
  connect(m_plugin, SIGNAL(regionRemoved(BoundingRegion*)),
          this, SLOT(regionRemoved(BoundingRegion*)));
}

//-----------------------------------------------------------------------------
RegionRenderer::~RegionRenderer()
{

}

//-----------------------------------------------------------------------------
void RegionRenderer::hide()
{
  if (!m_enable)
    return;

  m_enable = false;

  foreach(BoundingRegion *region, m_widgets.keys())
  {
    region->deleteWidget(m_widgets[region]);
  }
  m_widgets.clear();

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void RegionRenderer::show()
{
  if (m_enable)
    return;

  m_enable = true;
  vtkRenderWindow *rw = m_renderer->GetRenderWindow();
  vtkRenderWindowInteractor *interactor = rw->GetInteractor();

  foreach(BoundingRegion *region, m_plugin->regions())
  {
    m_widgets[region] = region->createWidget();
    m_widgets[region]->SetInteractor(interactor);
    m_widgets[region]->GetRepresentation()->SetVisibility(true);
    m_widgets[region]->On();
  }

  emit renderRequested();
}


//-----------------------------------------------------------------------------
void RegionRenderer::regionCreated(BoundingRegion* region)
{
  if (!m_enable)
    return;

  vtkRenderWindow *rw = m_renderer->GetRenderWindow();
  vtkRenderWindowInteractor *interactor = rw->GetInteractor();

  m_widgets[region] = region->createWidget();
  m_widgets[region]->SetInteractor(interactor);
  m_widgets[region]->GetRepresentation()->SetVisibility(true);
  m_widgets[region]->On();
}

//-----------------------------------------------------------------------------
void RegionRenderer::regionRemoved(BoundingRegion* region)
{
  if (!m_enable)
    return;

  region->deleteWidget(m_widgets[region]);
  m_widgets.remove(region);

}

//-----------------------------------------------------------------------------
Renderer* RegionRenderer::clone()
{
  RegionRenderer *rr = new RegionRenderer(m_plugin);
//   connect(m_plugin, SIGNAL(regionCreated(BoundingRegion*)),
//           rr, SLOT(regionCreated(BoundingRegion*)));
//   connect(m_plugin, SIGNAL(regionRemoved(BoundingRegion*)),
//           rr, SLOT(regionRemoved(BoundingRegion*)));
  return rr;
}