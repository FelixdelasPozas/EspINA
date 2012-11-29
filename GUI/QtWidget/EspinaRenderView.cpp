/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "EspinaRenderView.h"

#include <Core/Model/ModelItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>

#include <vtkMath.h>

#include <QApplication>
#include <QDebug>

//-----------------------------------------------------------------------------
EspinaRenderView::EspinaRenderView(QWidget* parent)
: QWidget(parent)
{
  m_sceneResolution[0] = m_sceneResolution[1] = m_sceneResolution[2] = 1;
  m_sceneBounds[0] = m_sceneBounds[2] = m_sceneBounds[4] = 0;
  m_sceneBounds[1] = m_sceneBounds[3] = m_sceneBounds[5] = 0;
}

//-----------------------------------------------------------------------------
EspinaRenderView::~EspinaRenderView()
{
}

//-----------------------------------------------------------------------------
void EspinaRenderView::previewBounds(Nm bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
  qDebug() << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
  bounds[0] = bounds[2] = bounds[4] =  0;
  bounds[1] = bounds[3] = bounds[5] = -1;
}


//-----------------------------------------------------------------------------
void EspinaRenderView::addChannelBounds(Channel* channel)
{
  Q_ASSERT(!m_channels.contains(channel));
  m_channels << channel;
  updateSceneBounds();
}

//-----------------------------------------------------------------------------
void EspinaRenderView::removeChannelBounds(Channel* channel)
{
  Q_ASSERT(m_channels.contains(channel));
  m_channels.removeOne(channel);
  updateSceneBounds();
}

//-----------------------------------------------------------------------------
double EspinaRenderView::suggestedChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(Channel * channel, m_channels)
    if (channel->isVisible())
      numVisibleRep++;

  if (numVisibleRep == 0)
    return 0.0;

  return 1.0 / numVisibleRep;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::resetSceneBounds()
{
  m_sceneResolution[0] = m_sceneResolution[1] = m_sceneResolution[2] = 1;
  m_sceneBounds[0] = m_sceneBounds[2] = m_sceneBounds[4] = 0;
  m_sceneBounds[1] = m_sceneBounds[3] = m_sceneBounds[5] = 0;
}

//-----------------------------------------------------------------------------
void EspinaRenderView::updateSceneBounds()
{
  if (!m_channels.isEmpty())
  {
    m_channels[0]->volume()->spacing(m_sceneResolution);
    m_channels[0]->volume()->bounds(m_sceneBounds);

    for(int c = 1; c < m_channels.size(); c++)
    {
      double spacing[3];
      double bounds[6];

      m_channels[c]->volume()->spacing(spacing);
      m_channels[c]->volume()->bounds(bounds);
      for (int i = 0; i < 3; i++)
      {
        m_sceneResolution[i] = std::min(m_sceneResolution[i], spacing[i]);
        m_sceneBounds[i] = std::min(m_sceneBounds[i], bounds[i]);
        m_sceneBounds[2*i+1] = std::max(m_sceneBounds[2*i+1], bounds[2*i+1]);
      }
    }
  }
  else
    resetSceneBounds();
}
