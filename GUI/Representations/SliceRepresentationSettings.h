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

#ifndef ESPINA_SLICEREPRESENTATIONSETTINGS_H
#define ESPINA_SLICEREPRESENTATIONSETTINGS_H

// ESPINA
#include "GUI/Representations/RepresentationSettings.h"

// Qt
#include "ui_SliceRepresentationSettings.h"

namespace ESPINA
{
  class EspinaGUI_EXPORT SliceRepresentationSettings
  : public RepresentationSettings
  , private Ui::SliceRepresentationSettings
  {
  public:
  	/* \brief SliceRepresentationSettings class constructor.
  	 *
  	 */
    explicit SliceRepresentationSettings();

    /* \brief Implements RepresentationSettings::get().
     *
     */
    virtual void get(RepresentationSPtr representation);

    /* \brief Implements RepresentationSettings::set().
     *
     */
    virtual void set(RepresentationSPtr representation);

  private:
    bool m_init;
  };

} // namespace ESPINA

#endif // ESPINA_SLICEREPRESENTATIONSETTINGS_H
