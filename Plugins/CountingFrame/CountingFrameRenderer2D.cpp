/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include <CountingFrameRenderer2D.h>
#include "CountingFrames/CountingFrame.h"
#include <GUI/View/Widgets/EspinaWidget.h>
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
  for(auto cf: m_insertedCFs)
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

  m_insertedCFs << cf;

  if (m_enable)
  {
    m_view->addWidget(cf);
    emit renderRequested();
  }
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::onCountingFrameDeleted(CountingFrame *cf)
{
  if (m_insertedCFs.contains(cf))
  {
    m_insertedCFs.removeOne(cf);

    if (m_enable)
    {
      m_view->removeWidget(cf);
      emit renderRequested();
    }
  }
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::hide()
{
  if (!m_enable)
    return;

  for(auto cf: m_insertedCFs)
    cf->setEnabled(false);

  if (!m_insertedCFs.empty())
    emit renderRequested();
}

//-----------------------------------------------------------------------------
void CountingFrameRenderer2D::show()
{
  if (m_enable)
    return;

  for(auto cf: m_insertedCFs)
    cf->setEnabled(true);

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