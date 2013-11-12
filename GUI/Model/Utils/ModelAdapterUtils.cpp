/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011
    Jorge Peña Pastor<jpena@cesvima.upm.es>,
    Felix de las Pozas<fpozas@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "ModelAdapterUtils.h"

#include <Filters/VolumetricStreamReader.h>
#include <Core/Analysis/Channel.h>

using namespace EspINA;

void EspINA::ModelAdapterUtils::setAnalysis(ModelAdapterSPtr model, AnalysisSPtr analysis, ModelFactorySPtr factory)
{
  if (analysis)
  {
    model->reset();

    QMap<FilterSPtr, FilterAdapterSPtr> filters;
    // Adaptar la clasificacion
    // Adaptar las muestras
    // Adaptar los canales --> adaptar sus filtros
    // Adaptar las segementaciones --> adaptar sus filtros
    //TODO Remove includes
    ChannelSPtr channelbase = analysis->channels().first();
//     std::shared_ptr<VolumetricStreamReader> base = std::dynamic_pointer_cast<VolumetricStreamReader>(channelbase->filter());
    FilterAdapterSPtr filter = filters.value(channelbase->filter(), FilterAdapterSPtr());

    if (!filter)
    {
      filter = factory->adaptFilter(channelbase->filter());
    }
    ChannelAdapterSPtr channel = factory->adaptChannel(filter, channelbase);

    model->add(channel);
  }
}

DefaultVolumetricDataSPtr EspINA::ModelAdapterUtils::volumetricData(OutputSPtr output)
{
  return std::dynamic_pointer_cast<VolumetricData<itkVolumeType>>(output->data(VolumetricData<itkVolumeType>::TYPE));
}
