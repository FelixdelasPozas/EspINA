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
#include <GUI/View/RenderView.h>


#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameRenderer::CountingFrameRenderer(CountingFrameManager& cfManager)
: m_cfManager(cfManager)
, m_cfCount(0)
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

  m_enable = false;

  for(auto cf : m_widgets.keys())
  {
    m_widgets[cf]->GetRepresentation()->SetVisibility(false);
    m_widgets[cf]->Off();
    //m_widgets[cf]->Delete();
  }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::show()
{
  if (m_enable)
    return;

  m_enable = true;
  auto rw         = m_view->renderWindow();
  auto interactor = rw->GetInteractor();

  for(auto cf : m_cfManager.countingFrames())
  {
    if (!m_widgets.contains(cf))
    {
      m_widgets[cf] = cf->create3DWidget(nullptr);
    }
    m_widgets[cf]->SetInteractor(interactor);
    m_widgets[cf]->GetRepresentation()->SetVisibility(true);
    m_widgets[cf]->On();

    auto cfWidget = dynamic_cast<vtkCountingFrame3DWidget *>(m_widgets[cf]);
    cfWidget->SetCountingFrameVisibility(true);
    cfWidget->SetEnabled(true);
  }

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
  m_cfCount++;

  setEnable(m_cfCount > 0);
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::onCountingFrameDeleted(CountingFrame *cf)
{
  m_cfCount--;

  if (m_widgets.contains(cf))
  {
    m_widgets[cf]->GetRepresentation()->SetVisibility(false);
    m_widgets[cf]->Off();
    m_widgets.remove(cf);
  }

  setEnable(m_cfCount > 0);
}

//-----------------------------------------------------------------------------
RendererSPtr CountingFrameRenderer::clone()
{
  return RendererSPtr(new CountingFrameRenderer(m_cfManager));
}
