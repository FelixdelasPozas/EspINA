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

#ifndef ROI_SETTINGS_H
#define ROI_SETTINGS_H

#include <App/Tools/ROI/RectangularROI.h>

namespace EspINA
{
  const QString DEFAULT_VOI_X("RectangularROI::DefaultROI::X");
  const QString DEFAULT_VOI_Y("RectangularROI::DefaultROI::Y");
  const QString DEFAULT_VOI_Z("RectangularROI::DefaultROI::Z");
  const QString ROI_SETTINGS_GROUP("Rectangular ROI Settins");

  class RectangularROI::ROISettings
  {
    public:
      explicit ROISettings();
      virtual ~ROISettings()
      {};

      void setXSize(int value);

      int xSize() const
      {return m_xSize;}

      void setYSize(int value);

      int ySize() const
      {return m_ySize;}

      void setZSize(int value);

      int zSize() const
      {return m_zSize;}

    private:
      int m_xSize, m_ySize, m_zSize;
  };

}

#endif // ROI_SETTINGS_H
