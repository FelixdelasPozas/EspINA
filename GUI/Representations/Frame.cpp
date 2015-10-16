/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "Frame.h"

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;

//------------------------------------------------------------------------
Frame::Frame()
: time(Timer::INVALID_TIME_STAMP)
, focus(false)
, reset(false)
{
}

//------------------------------------------------------------------------
FrameCSPtr Frame::InvalidFrame()
{
  return std::make_shared<Frame>();
}

//------------------------------------------------------------------------
bool Frame::isValid() const
{
  return time != Timer::INVALID_TIME_STAMP;
}

//------------------------------------------------------------------------
QDebug GUI::Representations::operator<<(QDebug d, const FrameSPtr frame)
{
  d << "Frame" << frame->time << "[" << frame->crosshair << "R:" << frame->reset << "F:" << frame->focus << "]";

  return d;
}

//------------------------------------------------------------------------
QDebug GUI::Representations::operator<<(QDebug d, const FrameCSPtr frame)
{
  d << "Frame" << frame->time << "[" << frame->crosshair << "R:" << frame->reset << "F:" << frame->focus << "]";

  return d;
}
