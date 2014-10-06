/*

    Copyright (C) 2014
    Jorge Pe�a Pastor<jpena@cesvima.upm.es>,
    Felix de las Pozas<fpozas@cesvima.upm.es>

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

#ifndef ESPINA_MODEL_ADAPTER_UTILS_H
#define ESPINA_MODEL_ADAPTER_UTILS_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  namespace ModelAdapterUtils
  {
//   	/** \brief Resets the model and adapts the elements in the analysis.
//   	 * \param[out] model, model adapter smart pointer.
//   	 * \param[in] analysis, analysis smart pointer.
//   	 * \param[in] factory, model factory smart pointer.
//   	 *
//   	 */
//     void EspinaGUI_EXPORT setAnalysis(ModelAdapterSPtr model, AnalysisSPtr analysis, ModelFactorySPtr factory);

  	/** \brief Returns the volumetric data associated with an output.
  	 * \param[in] output, smart pointer of the output with associated volumetric data.
  	 *
  	 */
    DefaultVolumetricDataSPtr EspinaGUI_EXPORT volumetricData(OutputSPtr output);

  	/** \brief Returns the first number for a segmentation not used in an analysis.
  	 * \param[in] model, model adapter smart pointer.
  	 *
  	 */
    unsigned int EspinaGUI_EXPORT firstUnusedSegmentationNumber(const ModelAdapterSPtr model);
  }
} // namespace ESPINA
#endif // ESPINA_MODEL_ADAPTER_UTILS_H
