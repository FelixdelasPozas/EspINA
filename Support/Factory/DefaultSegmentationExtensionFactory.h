/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_DEFAULT_SEGMENTATION_EXTENSION_FACTORY_H
#define ESPINA_DEFAULT_SEGMENTATION_EXTENSION_FACTORY_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Factory/SegmentationExtensionFactory.h>

namespace ESPINA {

  class EspinaSupport_EXPORT DefaultSegmentationExtensionFactory
  : public SegmentationExtensionFactory
  {
  public:
  	/* \brief Implements SegmentationExtensionFactory::createSegmentationExtension().
  	 *
  	 */
    virtual SegmentationExtensionSPtr createSegmentationExtension(const SegmentationExtension::Type      &type,
                                                                  const SegmentationExtension::InfoCache &cache = SegmentationExtension::InfoCache(),
                                                                  const State& state = State()) const;

    /* \brief Implements SegmentationExtensionFactory::providedExtensions().
     *
     */
    virtual SegmentationExtensionTypeList providedExtensions() const;
  };
} // namespace ESPINA

#endif // ESPINA_DEFAULT_SEGMENTATION_EXTENSION_FACTORY_H
