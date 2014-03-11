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

// Plugin
#include "CountingFrameRenderer3D.h"
#include "CountingFrames/CountingFrame.h"
#include <GUI/View/View3D.h>

// VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameRenderer3D::CountingFrameRenderer3D(CountingFrameManager& cfManager)
: m_cfManager(cfManager)
{
  connect(&m_cfManager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(onCountingFrameCreated(CountingFrame*)));
  connect(&m_cfManager, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this, SLOT(onCountingFrameDeleted(CountingFrame*)));

  m_enable = false;
}

//-----------------------------------------------------------------------------
CountingFrameRenderer3D::~CountingFrameRenderer3D()
{
  for(auto cf: m_widgets.keys())
    onCountingFrameDeleted(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::hide()
{
  if (!m_enable)
    return;

  bool updated = false;
  for(auto cf : m_widgets.keys())
  {
    if (m_widgets[cf] == nullptr)
      continue;

    bool visible = cf->isVisible();
    if (visible)
      m_widgets[cf]->SetEnabled(false);

    updated |= visible;
  }

  if (updated)
    emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::show()
{
  if (m_enable)
    return;

  bool updated = false;

  for(auto cf : m_widgets.keys())
  {
    if (m_widgets[cf] == nullptr)
      createWidget(cf);

    bool visible = cf->isVisible();
    if (visible)
    {
      m_widgets[cf]->SetEnabled(true);
      m_view->addActor(m_widgets[cf]->GetRepresentation());
    }

    updated |= visible;
  }

  if (updated)
    emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CountingFrameRenderer3D::numberOfvtkActors() const
{
  return m_enable? m_cfManager.countingFrames().size() * 2 : 0; // m_boundingRegion & m_representation vtkPolyDatas...
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::onCountingFrameCreated(CountingFrame *cf)
{
  if (!m_view)
    return;

  if (m_enable)
  {
    createWidget(cf);
    m_widgets[cf]->SetEnabled(true);
    m_view->addActor(m_widgets[cf]->GetRepresentation());
    emit renderRequested();
  }
  else
  {
    // lazy creation of the widget
    m_widgets[cf] = nullptr;
  }
  connect(cf, SIGNAL(changedVisibility()), this, SLOT(visibilityChanged()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::onCountingFrameDeleted(CountingFrame *cf)
{
  if (m_widgets.contains(cf))
  {
    if (m_widgets[cf] != nullptr)
    {
      bool visible = cf->isVisible();
      if (visible && m_enable)
      {
        m_view->removeActor(m_widgets[cf]->GetRepresentation());
        m_widgets[cf]->SetEnabled(false);
        emit renderRequested();
      }
    }
    m_widgets.remove(cf);
    disconnect(cf, SIGNAL(changedVisibility()), this, SLOT(visibilityChanged()));
  }
}

//-----------------------------------------------------------------------------
RendererSPtr CountingFrameRenderer3D::clone() const
{
  return RendererSPtr(new CountingFrameRenderer3D(m_cfManager));
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::visibilityChanged()
{
  if (!m_enable)
    return;

  auto cf = dynamic_cast<CountingFrame *>(sender());
  if (!m_widgets.keys().contains(cf) || !m_enable)
    return;

  bool visible = cf->isVisible();
  if (m_widgets[cf] != nullptr)
  {
    if (visible)
      m_view->addActor(m_widgets[cf]->GetRepresentation());
    else
      m_view->removeActor(m_widgets[cf]->GetRepresentation());
    m_widgets[cf]->SetEnabled(visible);
  }
  else
  {
    if (visible)
    {
      createWidget(cf);
      m_widgets[cf]->SetEnabled(true);
      m_view->addActor(m_widgets[cf]->GetRepresentation());
    }
  }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::setView(RenderView *view)
{
  m_view = view;

  auto existingCFs = m_cfManager.countingFrames();
  for(auto cf: existingCFs)
    onCountingFrameCreated(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer3D::createWidget(CountingFrame* cf)
{
  auto view3d     = dynamic_cast<View3D *>(m_view);
  auto rw         = m_view->renderWindow();
  auto interactor = rw->GetInteractor();

  m_widgets[cf] = cf->create3DWidget(view3d);
  m_widgets[cf]->SetInteractor(interactor);
}
