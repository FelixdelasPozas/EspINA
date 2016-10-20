/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// ESPINA
#include "AnalysisUtils.h"
#include <Core/Analysis/Category.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Factory/CoreFactory.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
ESPINA::AnalysisSPtr ESPINA::merge(AnalysisSPtr& lhs, AnalysisSPtr& rhs)
{
  auto mergedAnalysis = std::make_shared<Analysis>();
  mergedAnalysis->setStorage(lhs->storage());

  QMap<CategorySPtr, CategorySPtr> mergedCategory;

  QString classificationName1;
  QString classificationName2;

  CategorySList roots;

  if (lhs->classification())
  {
    classificationName1 = lhs->classification()->name();
    roots << lhs->classification()->root();
  }

  if (rhs->classification())
  {
    classificationName2 = rhs->classification()->name();
    roots << rhs->classification()->root();
  }

  QString classificationName;

  if (!classificationName1.isEmpty() && classificationName2.isEmpty())
  {
    classificationName = classificationName1;
  }
  else
  {
    if (classificationName1.isEmpty() && !classificationName2.isEmpty())
    {
      classificationName = classificationName2;
    }
    else
    {
      if (!classificationName1.isEmpty())
      {
        classificationName = QObject::tr("%1 %2 merge").arg(classificationName1).arg(classificationName2);
      }
    }
  }

  if (!roots.isEmpty())
  {
    auto classification = std::make_shared<Classification>(classificationName);
    for(auto root : roots)
    {
      CategorySList categories;
      categories << root->subCategories();
      while (!categories.isEmpty())
      {
        auto category = categories.takeFirst();
        try
        {
          mergedCategory[category] = classification->createNode(category->classificationName());
          mergedCategory[category]->setColor(category->color());
        }
        catch (const EspinaException &e)
        {
          mergedCategory[category] = classification->node(category->classificationName());
        }
        categories << category->subCategories();
      }
    }

    mergedAnalysis->setClassification(classification);
  }

  QMap<PersistentSPtr, PersistentSPtr> mergedItems;

  for(auto analysis : {lhs, rhs})
  {
    for(auto sample : analysis->samples())
    {
      auto mergedSample = findSample(sample, mergedAnalysis->samples());
      if (!mergedSample)
      {
        mergedSample = sample;
        mergedAnalysis->add(sample);
      }
      mergedItems[sample]   = mergedSample;
    }

    for(auto channel : analysis->channels())
    {
      // DESIGN: How to deal with different states (spacing) of same channels??
      auto mergedChannel = findChannel(channel, mergedAnalysis->channels());
      if (!mergedChannel)
      {
        mergedChannel = channel;
        mergedAnalysis->add(channel);
      }
      else
      {
        Q_ASSERT(mergedChannel->outputId() == channel->outputId());
        // Filters using channel output as input need to be updated
        auto filter       = channel->filter();
        auto mergedFilter = mergedChannel->filter();

        for(auto vertex : analysis->content()->successors(filter))
        {
          FilterSPtr succesor = std::dynamic_pointer_cast<Filter>(vertex);
          if (succesor)
          {
            InputSList updatedInputs;
            for (auto input : succesor->inputs())
            {
              if (input->filter() == filter)
              {
                Q_ASSERT(mergedFilter->validOutput(input->output()->id()));
                updatedInputs << getInput(mergedFilter, input->output()->id());
              }
              else
              {
                updatedInputs << input;
              }
            }
            succesor->setInputs(updatedInputs);
          }
        }

        // NOTE: Merges channel extensions. It wont merge different extensions' data (like different counting frames),
        // just adds missing extensions to the merged item.
        QStringList mergedItemExtensionsTypes;
        auto mergedItemExtensions = mergedChannel->extensions();
        for(auto mergedExtension: mergedItemExtensions)
        {
          mergedItemExtensionsTypes << mergedExtension->type();
        }

        for(auto channelExtension: channel->extensions())
        {
          if(!mergedItemExtensionsTypes.contains(channelExtension->type()))
          {
            mergedItemExtensions->add(channelExtension);
          }
        }
      }
      mergedItems[channel] = mergedChannel;
    }

    for(auto segmentation : analysis->segmentations())
    {
      auto category = segmentation->category();
      if (category)
      {
        segmentation->setCategory(mergedCategory[category]);
      }
      mergedAnalysis->add(segmentation);

      mergedItems[segmentation] = segmentation;
    }

    for(auto relation : analysis->relationships()->edges())
    {
      auto source = mergedItems[relation.source];
      auto target = mergedItems[relation.target];
      QString relationship(relation.relationship.c_str());

      try
      {
        mergedAnalysis->addRelation(source, target, relationship);
      }
      catch (const EspinaException &e)
      {
        // do nothing
      }
    }
  }

  lhs.reset();
  rhs.reset();

  return mergedAnalysis;
}

//-----------------------------------------------------------------------------
SampleSPtr ESPINA::findSample(SampleSPtr sample, SampleSList samples)
{
  for(auto result : samples)
  {
    if (sample->name() == result->name()) return result;
  }

  return SampleSPtr();
}

//-----------------------------------------------------------------------------
ChannelSPtr ESPINA::findChannel(ChannelSPtr channel, ChannelSList channels)
{
  for(auto result : channels)
  {
    if (channel->name() == result->name()) return result;
  }

  return ChannelSPtr();
}

//-----------------------------------------------------------------------------
unsigned int ESPINA::firstUnusedSegmentationNumber(const AnalysisSPtr analysis)
{
  unsigned int number = 0;

  for (auto segmentation: analysis->segmentations())
    if (segmentation->number() > number)
      number = segmentation->number();

  return ++number;
}
