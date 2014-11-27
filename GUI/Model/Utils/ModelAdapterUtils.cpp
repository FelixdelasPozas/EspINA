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

// ESPINA
#include "ModelAdapterUtils.h"
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>

// Qt
#include <QString>

using namespace ESPINA;

// //------------------------------------------------------------------------
// void ESPINA::ModelAdapterUtils::setAnalysis(ModelAdapterSPtr model, AnalysisSPtr analysis, ModelFactorySPtr factory)
// {
//   if (analysis)
//   {
//     model->reset();
//
//     QMap<SegmentationSPtr, SegmentationAdapterSPtr> segmentations;
//
//     QMap<PersistentSPtr, ItemAdapterSPtr> items;
//
//     // Adaptar la clasificacion
//     ClassificationAdapterSPtr classification{new ClassificationAdapter(analysis->classification())};
//     model->setClassification(classification);
//
//     // Adapt Samples
//     for(auto sample : analysis->samples())
//     {
//       auto adapted = factory->adaptSample(sample);
//       items[sample] = adapted;
//       model->add(adapted);
//     }
//     // Adapt channels
//     for(auto channel : analysis->channels())
//     {
//       auto adapted = factory->adaptChannel(channel);
//       items[channel] = adapted;
//       model->add(adapted);
//     }
//     // Adapt segmentation
//     for(auto segmentation : analysis->segmentations())
//     {
//       model->add(factory->adaptSegmentation(segmentation));
//     }
//
//     for(auto relation : analysis->relationships()->edges())
//     {
//       ItemAdapterSPtr source = items[relation.source];
//       ItemAdapterSPtr target = items[relation.target];
//       RelationName name(relation.relationship.c_str());
//
//       model->addRelation(source, target, name);
//     }
//   }
// }

//------------------------------------------------------------------------
DefaultVolumetricDataSPtr ESPINA::ModelAdapterUtils::volumetricData(OutputSPtr output)
{
  return std::dynamic_pointer_cast<VolumetricData<itkVolumeType>>(output->data(VolumetricData<itkVolumeType>::TYPE));
}

//------------------------------------------------------------------------
unsigned int ESPINA::ModelAdapterUtils::firstUnusedSegmentationNumber(const ModelAdapterSPtr model)
{
  unsigned int number = 0;

  for (auto segmentation: model->segmentations())
    if (segmentation->number() > number)
      number = segmentation->number();

  return ++number;
}

