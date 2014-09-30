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


#ifndef ESPINA_EXTENSION_FACTORY_H
#define ESPINA_EXTENSION_FACTORY_H

#include "Extensions/EspinaExtensions_Export.h"
#include <Core/Analysis/Extension.h>

namespace ESPINA
{
  class EspinaExtensions_EXPORT ExtensionFactory
  {
  public:
  	/** \brief ExtensionFactory class constructor.
  	 *
  	 */
    virtual ~ExtensionFactory() {}

    /** \brief Creates a channel extensions.
     * \param[in] type, channel extension type.
     *
     */
    virtual ChannelExtensionSPtr createChannelExtension(ChannelExtension::Type type) = 0;

    /** \brief Creates a segmentation extension.
     * \param[in] type, segmentation extenion type.
     *
     */
    virtual SegmentationExtensionSPtr createSegmentationExtension(SegmentationExtension::Type type) = 0;
  };

  using ExtensionFactorySPtr  = std::shared_ptr<ExtensionFactory>;
  using ExtensionFactorySList = QList<ExtensionFactorySPtr>;

}// namespace ESPINA

#endif // ESPINA_EXTENSION_FACTORY_H
