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

// ESPINA
#include "RepresentationWindow.h"

// C++
#include <cmath>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//-----------------------------------------------------------------------------
RepresentationWindow::RepresentationWindow(SchedulerSPtr              scheduler,
                                           RepresentationPipelineSPtr pipeline,
                                           unsigned int               windowSize)
: m_currentPos{windowSize}
, m_width     {windowSize}
{
  qRegisterMetaType<GUI::Representations::FrameCSPtr>("GUI::Representations::FrameCSPtr");
  qRegisterMetaType<RepresentationPipeline::Actors>("RepresentationPipelineActors");

  for (unsigned int i = 0; i < 2*windowSize + 1; ++i)
  {
    auto task = std::make_shared<RepresentationUpdater>(scheduler, pipeline);

    connect(task.get(), SIGNAL(actorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)),
            this,       SIGNAL(actorsReady(GUI::Representations::FrameCSPtr,RepresentationPipeline::Actors)), Qt::DirectConnection);

    m_buffer << task;
  }
}

//-----------------------------------------------------------------------------
QList<RepresentationWindow::Cursor> RepresentationWindow::moveCurrent(int distance)
{
  QList<Cursor> invalid;

  if (distance != 0)
  {
    if (abs(distance) >= m_buffer.size())
    {
      m_currentPos = m_width;

      for(unsigned int i = 0, j = -m_width; i < (2*m_width)+1; ++i, ++j)
      {
        invalid << Cursor(m_buffer[i],j);
      }
    }
    else
    {
      int n = abs(distance);
      int s = std::copysign(1.0, distance);
      int i = innerPosition(m_currentPos - s*m_width);

      while (n > 0)
      {
        int d = s*(m_width - n + 1);

        invalid << Cursor(m_buffer[i], d);

        i = (s > 0) ? nextPosition(i) : prevPosition(i);

        --n;
      }

      m_currentPos = innerPosition(m_currentPos + distance);
    }
  }

  return invalid;
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSPtr RepresentationWindow::current() const
{
  return m_buffer[m_currentPos];
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::all() const
{
  RepresentationUpdaterSList all;

  all << behind() << current() << ahead();

  return all;
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::behind() const
{
  return behindOf(m_currentPos, m_width);
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::ahead() const
{
  return aheadFrom(m_currentPos, m_width);
}


//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::closestBehind() const
{
  return behindOf(m_currentPos, closestDistance());
}

//-----------------------------------------------------------------------------

ESPINA::RepresentationUpdaterSList RepresentationWindow::closestAhead() const
{
  return aheadFrom(m_currentPos, closestDistance());
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::fartherBehind() const
{
  return behindOf(m_currentPos-closestDistance(), fartherDistance());
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::fartherAhead() const
{
  return aheadFrom(m_currentPos+closestDistance(), fartherDistance());
}

//-----------------------------------------------------------------------------
void RepresentationWindow::incrementBuffer()
{
  // TODO: increment buffer size on miss
}

//-----------------------------------------------------------------------------
void RepresentationWindow::decrementBuffer()
{
  // TODO: decrement buffer on status reset
}

//-----------------------------------------------------------------------------
int RepresentationWindow::size() const
{
  return m_buffer.size();
}

//-----------------------------------------------------------------------------
RepresentationUpdaterSList RepresentationWindow::aheadFrom(int pos, int length) const
{
  RepresentationUpdaterSList result;

  int i = innerPosition(pos);
  int l = length;

  while (l > 0)
  {
    i = nextPosition(i);

    result << m_buffer[i];

    --l;
  }

  return result;
}

//-----------------------------------------------------------------------------
RepresentationUpdaterSList RepresentationWindow::behindOf(int pos, int length) const
{
  RepresentationUpdaterSList result;

  int i = innerPosition(pos);
  int l = length;

  while (l > 0)
  {
    i = prevPosition(i);

    result << m_buffer[i];

    --l;
  }

  return result;
}

//-----------------------------------------------------------------------------
int RepresentationWindow::closestDistance() const
{
  return std::floor(m_width/2);
}

//-----------------------------------------------------------------------------
int RepresentationWindow::fartherDistance() const
{
  return m_width - closestDistance();
}

//-----------------------------------------------------------------------------
unsigned RepresentationWindow::nextPosition(int pos) const
{
  return (pos == m_buffer.size() - 1) ? 0 : pos + 1;
}

//-----------------------------------------------------------------------------
unsigned RepresentationWindow::prevPosition(int pos) const
{
  return (pos == 0) ? m_buffer.size() - 1 : pos - 1;
}

//-----------------------------------------------------------------------------
unsigned int RepresentationWindow::innerPosition(int pos) const
{
  int result = pos;

  if (pos < 0)
  {
    result = pos + m_buffer.size();
  }
  else
  {
    if (pos >= m_buffer.size())
    {
      result = pos - m_buffer.size();
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
QDebug ESPINA::operator<<(QDebug debug, const QList<RepresentationWindow::Cursor> &cursors)
{
  debug << "\n--- Window ------------------\n";
  for(auto cursor: cursors)
  {
    debug << "updater:" << cursor.first.get() << "pos:" << cursor.second << "\n";
  }
  debug << "\n-----------------------------\n";

  return debug;
}
