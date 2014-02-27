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
#include "CountingFrameRenderer.h"

#include "CountingFrames/CountingFrame.h"
#include <GUI/View/View3D.h>


#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameRenderer::CountingFrameRenderer(CountingFrameManager& cfManager)
: m_cfManager(cfManager)
{
  connect(&m_cfManager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(onCountingFrameCreated(CountingFrame*)));
  connect(&m_cfManager, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this, SLOT(onCountingFrameDeleted(CountingFrame*)));
}

//-----------------------------------------------------------------------------
CountingFrameRenderer::~CountingFrameRenderer()
{
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::hide()
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
void CountingFrameRenderer::show()
{
  if (m_enable)
    return;

  bool updated = false;
  auto volView = dynamic_cast<View3D *>(m_view);
  for(auto cf : m_widgets.keys())
  {
    if (m_widgets[cf] == nullptr)
    {
      auto rw         = m_view->renderWindow();
      auto interactor = rw->GetInteractor();

      m_widgets[cf] = cf->createWidget(volView);
      m_widgets[cf]->SetInteractor(interactor);
    }

    bool visible = cf->isVisible();
    if (visible)
    {
      m_widgets[cf]->SetEnabled(true);
      volView->addActor(m_widgets[cf]->GetRepresentation());
    }

    updated |= visible;
  }

  if (updated)
    emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CountingFrameRenderer::numberOfvtkActors()
{
  return m_enable? m_cfManager.countingFrames().size() * 2 : 0; // m_boundingRegion & m_representation vtkPolyDatas...
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::onCountingFrameCreated(CountingFrame *cf)
{
  if (!m_view)
    return;

  if (m_enable)
  {
    auto rw         = m_view->renderWindow();
    auto interactor = rw->GetInteractor();
    auto volView = dynamic_cast<View3D *>(m_view);

    m_widgets[cf] = cf->createWidget(volView);
    m_widgets[cf]->SetInteractor(interactor);

    m_widgets[cf]->SetEnabled(true);
    volView->addActor(m_widgets[cf]->GetRepresentation());
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
void CountingFrameRenderer::onCountingFrameDeleted(CountingFrame *cf)
{
  if (m_widgets.contains(cf))
  {
    if (m_widgets[cf] != nullptr)
    {
      bool visible = cf->isVisible();
      if (visible && m_enable)
      {
        auto volView = dynamic_cast<View3D *>(m_view);
        volView->removeActor(m_widgets[cf]->GetRepresentation());
        m_widgets[cf]->SetEnabled(false);
        emit renderRequested();
      }
    }
    m_widgets.remove(cf);
    disconnect(cf, SIGNAL(changedVisibility()), this, SLOT(visibilityChanged()));
  }
}

//-----------------------------------------------------------------------------
RendererSPtr CountingFrameRenderer::clone()
{
  return RendererSPtr(new CountingFrameRenderer(m_cfManager));
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::visibilityChanged()
{
  if (!m_enable)
    return;

  auto cf = dynamic_cast<CountingFrame *>(sender());
  if (!m_widgets.keys().contains(cf) || !m_enable)
    return;

  bool visible = cf->isVisible();
  if (m_widgets[cf] != nullptr)
  {
    auto volView = dynamic_cast<View3D *>(m_view);
    if (visible)
      volView->addActor(m_widgets[cf]->GetRepresentation());
    else
      volView->removeActor(m_widgets[cf]->GetRepresentation());
    m_widgets[cf]->SetEnabled(visible);
  }
  else
  {
    if (visible)
    {
      auto rw         = m_view->renderWindow();
      auto interactor = rw->GetInteractor();
      auto volView = dynamic_cast<View3D *>(m_view);

      m_widgets[cf] = cf->createWidget(volView);
      m_widgets[cf]->SetInteractor(interactor);
      m_widgets[cf]->SetEnabled(true);
      volView->addActor(m_widgets[cf]->GetRepresentation());
    }
  }

  emit renderRequested();
}
