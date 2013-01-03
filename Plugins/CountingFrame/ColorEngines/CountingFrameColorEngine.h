/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef COUNTINGFRAMECOLORENGINE_H
#define COUNTINGFRAMECOLORENGINE_H

#include <Core/ColorEngines/IColorEngine.h>

namespace EspINA
{

  class CountingFrameColorEngine
  : public ColorEngine
  {

  public:
    explicit CountingFrameColorEngine();

    virtual QColor color(SegmentationPtr seg);
    virtual LUTPtr lut(SegmentationPtr seg);
    virtual ColorEngine::Composition supportedComposition() const
    { return ColorEngine::Transparency; }

  private:
    LUTPtr m_discardedLUT;
    LUTPtr m_nonDiscartedLUT;
  };

} // namespace EspINA

#endif // COUNTINGFRAMECOLORENGINE_H