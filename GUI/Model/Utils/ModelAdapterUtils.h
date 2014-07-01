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

#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <Core/Analysis/Data/VolumetricData.h>

namespace EspINA
{
  namespace ModelAdapterUtils
  {
    void setAnalysis(ModelAdapterSPtr model, AnalysisSPtr analysis, ModelFactorySPtr factory);

    DefaultVolumetricDataSPtr volumetricData(OutputSPtr output);

    unsigned int firstUnusedSegmentationNumber(const ModelAdapterSPtr model);
  }
} // namespace EspINA
#endif // ESPINA_MODEL_ADAPTER_UTILS_H
