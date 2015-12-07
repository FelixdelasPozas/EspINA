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

#ifndef ESPINA_SEGMENTATION_EXTENSION_FACTORY_H
#define ESPINA_SEGMENTATION_EXTENSION_FACTORY_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  /** \class SegmentationExtensionFactory
   * \brief Extension factory for segmentation objects.
   *
   */
  class EspinaCore_EXPORT SegmentationExtensionFactory
  {
    public:
      /** \brief SegmentationExtensionFactory class constructor.
       *
       */
      virtual ~SegmentationExtensionFactory()
      {}

      /** \brief Creates a segmentation extension of the given type with the given state and cache object.
       * \param[in] type, segmentation extension type.
       * \param[in] cache, information cache object.
       * \param[in] state, state object.
       *
       */
      virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                    const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache() ,
                                                                    const State                            &state = State()) const = 0;

      /** \brief Returns the list of types of segmentation extensions this filter can create.
       *
       */
      virtual SegmentationExtensionTypeList providedExtensions() const = 0 ;
  };

  using SegmentationExtensionFactoryPtr   = SegmentationExtensionFactory *;
  using SegmentationExtensionFactorySPtr  = std::shared_ptr<SegmentationExtensionFactory>;
  using SegmentationExtensionFactorySList = QList<SegmentationExtensionFactorySPtr>;

}// namespace ESPINA

#endif // ESPINA_SEGMENTATION_EXTENSION_FACTORY_H
