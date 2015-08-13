/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_SELECTION_UTILS_H
#define ESPINA_SELECTION_UTILS_H

#include "Support/EspinaSupport_Export.h"

#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/Selection.h>

using namespace ESPINA::GUI::View;

namespace ESPINA
{
  /** \brief Returns the list of current selected segmentations
   *  \param[in] viewManager ViewManager where current selection is gotten from
   */
  SegmentationAdapterList EspinaSupport_EXPORT selectSegmentations(SelectionSPtr selection);
}

#endif // ESPINA_SELECTION_UTILS_H
