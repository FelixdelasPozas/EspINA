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

#include "CountingFramePanel.h"
#include "CountingFrames/CountingFrame.h"

#include <Core/Model/ModelItem.h>

#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>
#include <GUI/QtWidget/EspinaRenderView.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CountingFrameRenderer::CountingFrameRenderer(CountingFramePanel* plugin)
: m_plugin(plugin)
, m_cfCount(0)
{
  connect(m_plugin, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(countingFrameCreated(CountingFrame*)));
  connect(m_plugin, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this, SLOT(countingFrameDeleted(CountingFrame*)));
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

  foreach(CountingFrame *cf, m_widgets.keys())
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
  vtkRenderWindow *rw = m_view->renderWindow();
  vtkRenderWindowInteractor *interactor = rw->GetInteractor();

  foreach(CountingFrame *cf, m_plugin->countingFrames())
  {
    if (!m_widgets.contains(cf))
      m_widgets[cf] = cf->create3DWidget(NULL);
    m_widgets[cf]->SetInteractor(interactor);
    m_widgets[cf]->GetRepresentation()->SetVisibility(true);
    m_widgets[cf]->On();
    vtkCountingFrame3DWidget *cfWidget = dynamic_cast<vtkCountingFrame3DWidget *>(m_widgets[cf]);
    cfWidget->SetCountingFrameVisibility(true);
    cfWidget->SetEnabled(true);
  }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CountingFrameRenderer::getNumberOfvtkActors()
{
  if (m_enable)
    return m_plugin->countingFrames().size() * 2; // m_boundingRegion & m_representation vtkPolyDatas...

  return 0;
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::countingFrameCreated(CountingFrame* cf)
{
  m_cfCount++;
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::countingFrameDeleted(CountingFrame* cf)
{
  m_cfCount--;

  if (m_widgets.contains(cf))
  {
    m_widgets[cf]->GetRepresentation()->SetVisibility(false);
    m_widgets[cf]->Off();
    m_widgets.remove(cf);
  }

  if (0 == m_cfCount)
    setEnable(false);
}

//-----------------------------------------------------------------------------
IRendererSPtr CountingFrameRenderer::clone()
{
  return IRendererSPtr(new CountingFrameRenderer(m_plugin));
}
