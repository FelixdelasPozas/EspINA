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

namespace ESPINA
{
  const QString DEFAULT_ROI_X("DefaultOrthogonalROI::X");
  const QString DEFAULT_ROI_Y("DefaultOrthogonalROI::Y");
  const QString DEFAULT_ROI_Z("DefaultOrthogonalROI::Z");
  const QString ROI_SETTINGS_GROUP("Orthogonal ROI Settings");

  class ROISettings
  {
    public:
  		/* \brief Class ROISettings class constructor.
  		 *
  		 */
      explicit ROISettings();

      /* \brief Class ROISettings class virtual destructor.
       *
       */
      virtual ~ROISettings()
      {};

      /* \brief Sets X size.
       * \param[in] value, X size value.
       *
       */
      void setXSize(int value);

      /* \brief Returns the value of the X size.
       *
       */
      int xSize() const
      {return m_xSize;}

      /* \brief Sets Y size.
       * \param[in] value, Y size value.
       *
       */
      void setYSize(int value);

      /* \brief Returns the value of the Y size.
       *
       */
      int ySize() const
      {return m_ySize;}

      /* \brief Sets Z size.
       * \param[in] value, Z size value.
       *
       */
      void setZSize(int value);

      /* \brief Returns the value of the Z size.
       *
       */
      int zSize() const
      {return m_zSize;}

    private:
      int m_xSize, m_ySize, m_zSize;
  };

}

#endif // ESPINA_ROI_SETTINGS_H
