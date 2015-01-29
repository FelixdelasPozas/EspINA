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

#include "RepresentationWindow.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
RepresentationWindow::RepresentationWindow(SchedulerSPtr scheduler, unsigned windowSize)
: m_scheduler(scheduler)
, m_currentPos(windowSize)
, m_witdh(windowSize)
{
  for (int i = 0; i < 2*windowSize +1; ++i)
  {
    m_buffer << std::make_shared<RepresentationUpdater>(scheduler);
  }
}

//-----------------------------------------------------------------------------
QList<RepresentationWindow::Cursor> RepresentationWindow::moveCurrent(int distance)
{
  QList<Cursor> invalid;

  if (distance > m_buffer.size())
  {
    m_currentPos = m_witdh;

    for (int i = 0, d = m_witdh; d > 0; ++i, --d)
    {
      invalid << Cursor(m_buffer[i], d);
      invalid << Cursor(m_buffer[m_witdh+i], i);
    }
    invalid << Cursor(m_buffer.last(), m_witdh);
  }
  else
  {
    if (distance > 0)
    {
      int d = m_witdh - distance;
      int i = innerPosition(m_currentPos - m_witdh);

      while (d > 0)
      {
        invalid << Cursor(m_buffer[i], d);

        i = nextPosition(i);
        --d;
      }
    }
    else if (distance < 0)
    {
      int d = m_witdh;
      int i = innerPosition(m_currentPos + m_witdh - distance);

      while (d > abs(distance))
      {
        invalid << Cursor(m_buffer[i], d);

        i = nextPosition(i);
        --d;
      }
    }

    m_currentPos += distance;
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
  return m_buffer;
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::behind() const
{
  return behindOf(m_currentPos, m_witdh);
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::ahead() const
{
  return aheadFrom(m_currentPos, m_witdh);
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
ESPINA::RepresentationUpdaterSList RepresentationWindow::furtherBehind() const
{
  return behindOf(m_currentPos-closestDistance(), furtherDistance());
}

//-----------------------------------------------------------------------------
ESPINA::RepresentationUpdaterSList RepresentationWindow::furtherAhead() const
{
  return aheadFrom(m_currentPos+closestDistance(), furtherDistance());
}

//-----------------------------------------------------------------------------
void RepresentationWindow::incrementBuffer()
{
}

//-----------------------------------------------------------------------------
void RepresentationWindow::decrementBuffer()
{

}

//-----------------------------------------------------------------------------
void RepresentationWindow::size() const
{
  return m_buffer.size();
}

//-----------------------------------------------------------------------------
RepresentationUpdaterSList RepresentationWindow::aheadFrom(int pos, int length) const
{
  RepresentationUpdaterSList result;

  int i = pos;
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

  int i = pos;
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
  return m_witdh/2;
}

//-----------------------------------------------------------------------------
int RepresentationWindow::furtherDistance() const
{
  return m_witdh - closestDistance();
}

//-----------------------------------------------------------------------------
unsigned RepresentationWindow::nextPosition(int pos) const
{
  return (pos == m_buffer.size() - 1)?0:pos + 1;
}

//-----------------------------------------------------------------------------
unsigned RepresentationWindow::prevPosition(int pos) const
{
  return (pos == 0)?m_buffer.size() - 1:pos - 1;
}

unsigned int RepresentationWindow::innerPosition(int pos) const
{
  int result = pos;

  if (pos < 0)
  {
    result = pos + m_buffer.size();
  }
  else if (pos >= m_buffer.size())
  {
    result = pos - m_buffer.size();
  }
}
