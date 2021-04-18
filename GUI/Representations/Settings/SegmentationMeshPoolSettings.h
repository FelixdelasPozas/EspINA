/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONMESHPOOLSETTINGS_H_
#define GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONMESHPOOLSETTINGS_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  /** \class SegmentationMeshPoolSettins
   * \brief Settings for the pool of mesh representations.
   *
   */
  class EspinaGUI_EXPORT SegmentationMeshPoolSettings
  : public PoolSettings
  {
    public:
      static const QString SMOOTH_KEY;

      /** \brief SegmentationMeshPoolSettings class constructor.
       *
       */
      SegmentationMeshPoolSettings();

      /** \brief SegmentationMeshPoolSettings class virtual destructor.
       *
       */
      virtual ~SegmentationMeshPoolSettings();

      /** \brief Sets the smooth value.
       * \param[in] value.
       */
      void setSmoothValue(int value);

      /** \brief Returns the smooth value.
       *
       */
      int smoothValue() const;
  };

} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_SETTINGS_SEGMENTATIONMESHPOOLSETTINGS_H_
