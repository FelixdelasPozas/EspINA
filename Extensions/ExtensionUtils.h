/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <GUI/Model/SegmentationAdapter.h>

namespace EspINA {

  template<typename Extension, typename Extendible>
  std::shared_ptr<Extension> retrieveOrCreateExtension(Extendible item)
  {
    std::shared_ptr<Extension> extension;

    if (!item->hasExtension(Extension::TYPE))
    {
      extension = std::shared_ptr<Extension>{new Extension()};
        item->addExtension(extension);
    } else
    {
      auto base = item->extension(Extension::TYPE);
      extension = std::dynamic_pointer_cast<Extension>(base);
    }

    Q_ASSERT(extension);

    return extension;
  };

} // ESPINA

#endif // ESPINA_CF_EXTENSION_UTILS_H
