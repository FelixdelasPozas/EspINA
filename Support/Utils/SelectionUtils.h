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
#include <Support/ViewManager.h>

namespace ESPINA
{
  /** \brief Returns the list of current selected segmentations
   *  \param[in] viewManager ViewManager where current selection is gotten from
   */
  SegmentationAdapterList EspinaSupport_EXPORT selectSegmentations(ViewManagerSPtr viewManager);

  /** \brief Returns the list of current selected segmentations or all existing segmentations if
   *         none are selected
   *  \param[in] viewmanager viewmanager where current selection is gotten from
   *  \param[in] model where segmentations are selected from in case no selection is available
   */
  SegmentationAdapterList EspinaSupport_EXPORT defaultReportInputSegmentations(ViewManagerSPtr viewManager, ModelAdapterSPtr model);
}

#endif // ESPINA_SELECTION_UTILS_H