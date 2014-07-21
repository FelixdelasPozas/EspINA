/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_ROI_SETTINGS_H
#define ESPINA_ROI_SETTINGS_H

#include <QString>

namespace EspINA
{
  const QString DEFAULT_ROI_X("DefaultOrthogonalROI::X");
  const QString DEFAULT_ROI_Y("DefaultOrthogonalROI::Y");
  const QString DEFAULT_ROI_Z("DefaultOrthogonalROI::Z");
  const QString ROI_SETTINGS_GROUP("Orthogonal ROI Settings");

  class ROISettings
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

#endif // ESPINA_ROI_SETTINGS_H
