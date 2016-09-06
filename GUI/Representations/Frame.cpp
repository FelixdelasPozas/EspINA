/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include "Frame.h"

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//------------------------------------------------------------------------
Frame::Frame()
: time      {Timer::INVALID_TIME_STAMP}
, crosshair {NmVector3{0,0,0}}
, resolution{NmVector3{1,1,1}}
, bounds    {Bounds{0,1,0,1,0,1}}
, flags     {Options()}
{
}

//------------------------------------------------------------------------
Frame::~Frame()
{
  //qDebug() << "destroy frame" << time;
}

//------------------------------------------------------------------------
FrameCSPtr Frame::InvalidFrame()
{
  static FrameCSPtr invalidFrame = std::make_shared<Frame>();

  return invalidFrame;
}

//------------------------------------------------------------------------
bool GUI::Representations::isValid(const FrameCSPtr frame)
{
  return frame->time != Timer::INVALID_TIME_STAMP;
}

//------------------------------------------------------------------------
bool GUI::Representations::requiresFocus(const FrameCSPtr frame)
{
  return frame->flags.testFlag(Frame::Focus);
}

//------------------------------------------------------------------------
bool GUI::Representations::requiresReset(const FrameCSPtr frame)
{
  return frame->flags.testFlag(Frame::Reset);
}

//------------------------------------------------------------------------
bool GUI::Representations::invalidatesSegmentations(const FrameCSPtr frame)
{
  return frame->flags.testFlag(Frame::InvalidateSegmentation);
}

//------------------------------------------------------------------------
bool GUI::Representations::invalidatesChannels(const FrameCSPtr frame)
{
  return frame->flags.testFlag(Frame::InvalidateChannel);
}

//------------------------------------------------------------------------
QDebug GUI::Representations::operator<<(QDebug d, const FrameSPtr frame)
{
  d << "Frame" << frame->time
    << "["
    << frame->crosshair
    << "R:" << frame->flags.testFlag(Frame::Reset)
    << "F:" << frame->flags.testFlag(Frame::Focus)
    << "IS:" << frame->flags.testFlag(Frame::InvalidateSegmentation)
    << "IC:" << frame->flags.testFlag(Frame::InvalidateChannel)
    << "]";

  return d;
}

//------------------------------------------------------------------------
QDebug GUI::Representations::operator<<(QDebug d, const FrameCSPtr frame)
{
  d << "Frame" << frame->time
    << "["
    << frame->crosshair
    << "R:" << frame->flags.testFlag(Frame::Reset)
    << "F:" << frame->flags.testFlag(Frame::Focus)
    << "IS:" << frame->flags.testFlag(Frame::InvalidateSegmentation)
    << "IC:" << frame->flags.testFlag(Frame::InvalidateChannel)
    << "]";

  return d;
}
