/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SEGMENTATION_SLICE_POOL_SETTINGS_H_
#define ESPINA_SEGMENTATION_SLICE_POOL_SETTINGS_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  /** \class SegmentationSlicePoolSettings
   * \brief Settings for the pool of slice representations.
   *
   */
  class EspinaGUI_EXPORT SegmentationSlicePoolSettings
  : public PoolSettings
  {
    public:
      /** \brief SegmentationPoolSettins class constructor.
       *
       */
      SegmentationSlicePoolSettings();

      /** \brief Sets the opacity setting value.
       * \param[in] value numerical value in [0,1].
       *
       */
      void setOpacity(double value);

      /** \brief Returns the value of the opacity setting.
       *
       */
      double opacity() const;
  };

  using SegmentationSlicePoolSettingsSPtr = std::shared_ptr<SegmentationSlicePoolSettings>;

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_SLICE_POOL_SETTINGS_H_
