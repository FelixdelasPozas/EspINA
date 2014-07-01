/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include <CountingFrameRenderer2D.h>
#include "CountingFrames/CountingFrame.h"
#include "CountingFrames/vtkCountingFrameSliceWidget.h"
#include <GUI/View/View2D.h>


using namespace EspINA;
using namespace EspINA::CF;

//-----------------------------------------------------------------------------
CountingFrameRenderer2D::CountingFrameRenderer2D(CountingFrameManager& cfManager)
: m_cfManager(cfManager)
{
  connect(&m_cfManager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this, SLOT(onCountingFrameCreated(CountingFrame*)));
  connect(&m_cfManager, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this, SLOT(onCountingFrameDeleted(CountingFrame*)));

  m_enable = true;
}

//-----------------------------------------------------------------------------
CountingFrameRenderer2D::~CountingFrameRenderer2D()
{
  for(auto cf: m_insertedCFs.keys())
    onCountingFrameDeleted(cf);
}

//-----------------------------------------------------------------------------
unsigned int CountingFrameRenderer2D::numberOfvtkActors() const
{
  return m_enable? m_cfManager.countingFrames().size() * 2 : 0; // m_boundingRegion & m_representation vtkPolyDatas...
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::onCountingFrameCreated(CountingFrame *cf)
{
  if (!m_view)
    return;

  cf->registerView(m_view);
  auto widget = dynamic_cast<vtkCountingFrameSliceWidget *>(cf->getWidget(m_view));
  Q_ASSERT(widget);
  widget->SetEnabled(m_enable);
  m_insertedCFs.insert(cf, widget);

  if (m_enable)
    emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::onCountingFrameDeleted(CountingFrame *cf)
{
  if(m_insertedCFs.keys().contains(cf))
  {
    m_insertedCFs.remove(cf);
    if(m_enable)
      emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::hide()
{
  if (!m_enable)
    return;

  for(auto widget: m_insertedCFs.values())
    widget->SetEnabled(false);

  if (!m_insertedCFs.empty())
    emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::show()
{
  if (m_enable)
    return;

  for(auto widget: m_insertedCFs.values())
    widget->SetEnabled(true);

  if (!m_insertedCFs.empty())
    emit renderRequested();
}

//-----------------------------------------------------------------------------
RendererSPtr CountingFrameRenderer2D::clone() const
{
  return RendererSPtr(new CountingFrameRenderer2D(m_cfManager));
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::setView(RenderView *view)
{
  m_view = view;

  auto existingCFs = m_cfManager.countingFrames();
  for(auto cf: existingCFs)
    onCountingFrameCreated(cf);
}
