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

#ifndef ESPINA_EXTENSION_UTILS_H
#define ESPINA_EXTENSION_UTILS_H

// ESPINA
#include <Core/Factory/CoreFactory.h>
#include <Core/Factory/ExtensionFactory.h>
#include <Extensions/LibraryExtensionFactory.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/ModelFactory.h>

namespace ESPINA
{
  namespace Extensions
  {
    /** \brief Templatized extension retrieval. The extended item must
     * be extended by the templated extension.
     * \param[in] extensions extension list locked object.
     *
     */
    template<typename Extension, typename Extensions>
    std::shared_ptr<Extension> retrieveExtension(Extensions extensions)
    {
      Q_ASSERT(extensions->hasExtension(Extension::TYPE));

      auto base      = extensions[Extension::TYPE];
      auto extension = std::dynamic_pointer_cast<Extension>(base);

      Q_ASSERT(extension);

      return extension;
    };

    /** \brief Safe delete extension from item
     * \param[in] item to get the extension deleted from
     *
     */
    template<typename Extension, typename Extensible>
    void safeDeleteExtension(Extensible item)
    {
      auto extensions = item->extensions();

      if (extensions->hasExtension(Extension::TYPE))
      {
        extensions->remove(Extension::TYPE);
      }
    }
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_CF_EXTENSION_UTILS_H
