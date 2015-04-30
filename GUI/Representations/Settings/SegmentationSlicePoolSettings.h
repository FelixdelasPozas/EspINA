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

// ESPINA
#include <GUI/Representations/RepresentationPool.h>

namespace ESPINA
{
  class SegmentationSlicePoolSettings
  : public RepresentationPool::Settings
  {
    public:
      explicit SegmentationSlicePoolSettings();

      void setOpacity(double value);

      double opacity() const;

    private:
      virtual RepresentationState poolSettingsImplementation() const override;

      double m_opacity;
  };

  using SegmentationSlicePoolSettingsSPtr = std::shared_ptr<SegmentationSlicePoolSettings>;

} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_SLICE_POOL_SETTINGS_H_
