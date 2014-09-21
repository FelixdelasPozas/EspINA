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

#ifndef ESPINA_GUI_DEFAULTICONS_H
#define ESPINA_GUI_DEFAULTICONS_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QIcon>

namespace ESPINA
{
  namespace GUI
  {
    class EspinaGUI_EXPORT DefaultIcons
    {
    public:
    	/** brief Return the system default save icon.
    	 *
    	 */
      static QIcon Save();

    	/** brief Return the system default load icon.
    	 *
    	 */
      static QIcon Load();

    	/** brief Return the system default file icon.
    	 *
    	 */
      static QIcon File();
    };

  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_DEFAULTICONS_H
