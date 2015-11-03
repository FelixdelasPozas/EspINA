/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Plugin
#include "RepresentationManager3D.h"

// ESPINA
#include <GUI/View/RenderView.h>

using namespace ESPINA;
using namespace ESPINA::CF;
using namespace ESPINA::GUI::Representations;

//-----------------------------------------------------------------------------
RepresentationManager3D::RepresentationManager3D(CountingFrameManager &manager, ViewTypeFlags supportedViews)
: RepresentationManager(supportedViews, RepresentationManager::EXPORTS_3D|RepresentationManager::NEEDS_ACTORS)
, m_manager(manager)
{
  connect(&m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this,       SLOT(onCountingFrameCreated(CountingFrame*)));

  connect(&m_manager, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this,       SLOT(onCountingFrameDeleted(CountingFrame*)));
}

//-----------------------------------------------------------------------------
RepresentationManager3D::~RepresentationManager3D()
{
  for(auto cf : m_widgets.keys())
  {
    deleteWidget(cf);
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationManager3D::pick(const NmVector3 &point, vtkProp *actor) const
{
  return ViewItemAdapterList();
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::onCountingFrameCreated(CountingFrame *cf)
{
  if (isActive())
  {
    auto widget = createWidget(cf);

    m_widgets.insert(cf, widget);

    emitRenderRequest(m_view->state().createFrame());
  }
  else
  {
    m_pendingCFs << cf;
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::onCountingFrameDeleted(CountingFrame *cf)
{
  if (m_pendingCFs.contains(cf))
  {
    m_pendingCFs.removeOne(cf);
  }
  else
  {
    Q_ASSERT(m_widgets.keys().contains(cf));

    deleteWidget(cf);

    emitRenderRequest(m_view->state().createFrame());
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager3D::hasRepresentations() const
{
  return !m_widgets.isEmpty();
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::updateFrameRepresentations(const FrameCSPtr frame)
{
  emitRenderRequest(frame);
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::onShow(const FrameCSPtr frame)
{
  for (auto cf : m_pendingCFs)
  {
    m_widgets[cf] = createWidget(cf);
  }

  m_pendingCFs.clear();
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::onHide(const FrameCSPtr frame)
{
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::displayRepresentations(const FrameCSPtr frame)
{
  for (auto widget : m_widgets)
  {
    showWidget(widget);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::hideRepresentations(const FrameCSPtr frame)
{
  for (auto widget : m_widgets)
  {
    hideWidget(widget);
  }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager3D::cloneImplementation()
{
  return std::make_shared<RepresentationManager3D>(m_manager, supportedViews());
}

//-----------------------------------------------------------------------------
vtkCountingFrame3DWidget *RepresentationManager3D::createWidget(CountingFrame *cf)
{
  auto widget = cf->createWidget(m_view);

  return widget;
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::showWidget(vtkCountingFrameWidget *widget)
{
  widget->SetEnabled(true);
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::hideWidget(vtkCountingFrameWidget *widget)
{
  widget->SetEnabled(false);
}

//-----------------------------------------------------------------------------
void RepresentationManager3D::deleteWidget(CountingFrame *cf)
{
  auto widget = m_widgets[cf];

  hideWidget(widget);

  cf->deleteWidget(widget);

  m_widgets.remove(cf);
}
