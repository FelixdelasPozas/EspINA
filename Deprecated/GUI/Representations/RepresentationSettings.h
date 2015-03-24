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

#ifndef ESPINA_REPRESENTATION_SETTINGS_H
#define ESPINA_REPRESENTATION_SETTINGS_H

// ESPINA
#include "Representation.h"

// Qt
#include <QWidget>

namespace ESPINA
{
  class EspinaGUI_EXPORT RepresentationSettings
  : public QWidget
  {
  public:
  	/** \brief RepresentationSettings class virtual destructor.
  	 *
  	 */
    virtual ~RepresentationSettings()
    {}

    /** \brief Gets the parameters of the representation and configures the widget with them.
     * \param[in] rep, representation smart pointer.
     *
     */
    virtual void get(RepresentationSPtr representation) = 0;

    /** \brief Configures the representation with the values in the widget.
     * \param[in] rep, representation smart pointer.
     *
     */
    virtual void set(RepresentationSPtr representation) = 0;
  };

} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_SETTINGS_H
