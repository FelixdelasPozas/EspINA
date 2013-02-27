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

using namespace EspINA;

//-----------------------------------------------------------------------------
CountingFrameRenderer::CountingFrameRenderer(CountingFramePanel* plugin)
: m_plugin(plugin)
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
    //cf->deleteWidget(m_widgets[cf]);
    m_widgets[cf]->GetRepresentation()->SetVisibility(false);
    m_widgets[cf]->Off();
  }
  m_widgets.clear();

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::show()
{
  if (m_enable)
    return;

  m_enable = true;
  vtkRenderWindow *rw = m_renderer->GetRenderWindow();
  vtkRenderWindowInteractor *interactor = rw->GetInteractor();

  foreach(CountingFrame *cf, m_plugin->countingFrames())
  {
//     m_widgets[cf] = cf->create3DWidget();
//     m_widgets[cf]->SetInteractor(interactor);
    m_widgets[cf]->GetRepresentation()->SetVisibility(true);
    m_widgets[cf]->On();
  }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CountingFrameRenderer::getNumberOfvtkActors()
{
  return 0;
}


//-----------------------------------------------------------------------------
void CountingFrameRenderer::countingFrameCreated(CountingFrame* cf)
{
  //TODO: BUUUG!!
//   if (!m_enable)
//     return;
// 
//   vtkRenderWindow *rw = m_renderer->GetRenderWindow();
//   vtkRenderWindowInteractor *interactor = rw->GetInteractor();
// 
//   m_widgets[cf] = cf->create3DWidget();
//   m_widgets[cf]->SetInteractor(interactor);
//   m_widgets[cf]->GetRepresentation()->SetVisibility(true);
//   m_widgets[cf]->On();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::countingFrameDeleted(CountingFrame* cf)
{
  // TODO: BUUUG!!
//   if (!m_enable)
//     return;
// 
//   // TODO: Review it is already deleted
//   m_widgets[cf]->GetRepresentation()->SetVisibility(false);
//   m_widgets[cf]->Off();
//   //cf->deleteWidget(m_widgets[cf]);
//   m_widgets.remove(cf);
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer::clean()
{
  // TODO 2012-12-29
}

//-----------------------------------------------------------------------------
IRendererSPtr CountingFrameRenderer::clone()
{
  return IRendererSPtr(new CountingFrameRenderer(m_plugin));
}