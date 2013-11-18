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

#include "AnalysisUtils.h"

#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Channel.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
EspINA::AnalysisSPtr EspINA::merge(AnalysisSPtr& lhs, AnalysisSPtr& rhs)
{
  AnalysisSPtr mergedAnalysis{new Analysis()};

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
  } else if (classificationName1.isEmpty() && !classificationName2.isEmpty())
  {
    classificationName = classificationName2;
  } else if (!classificationName1.isEmpty())
  {
    classificationName = QObject::tr("%1 %2 merge").arg(classificationName1).arg(classificationName2);
  }

  if (!roots.isEmpty())
  {
    ClassificationSPtr classification{new Classification(classificationName)};
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
        } catch (Already_Defined_Node_Exception e)
        {
          mergedCategory[category] = classification->node(category->classificationName());
        }
        categories << category->subCategories();
      }
    }

    mergedAnalysis->setClassification(classification);
  }

  QMap<SampleSPtr, SampleSPtr>   mergedSamples;
  QMap<ChannelSPtr, ChannelSPtr> mergedChannels;

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
      mergedSamples[sample] = mergedSample;
    }

    for(auto channel : analysis->channels())
    {
      auto mergedChannel = findChannel(channel, mergedAnalysis->channels());
      if (!mergedChannel)
      {
        mergedChannel = channel;
        mergedAnalysis->add(channel);
      }
      mergedChannels[channel] = mergedChannel;
    }

    for(auto segmentation : analysis->segmentations())
    {
      auto category = segmentation->category();
      if (category)
      {
        segmentation->setCategory(mergedCategory[category]);
      }
      mergedAnalysis->add(segmentation);
    }
  }

  // TODO: Relationships

  lhs.reset();
  rhs.reset();

  return mergedAnalysis;
}

//-----------------------------------------------------------------------------
SampleSPtr EspINA::findSample(SampleSPtr sample, SampleSList samples)
{
  for(auto result : samples)
  {
    if (sample->name() == result->name()) return result;
  }

  return SampleSPtr();
}

//-----------------------------------------------------------------------------
ChannelSPtr EspINA::findChannel(ChannelSPtr channel, ChannelSList channels)
{
  for(auto result : channels)
  {
    if (channel->name() == result->name()) return result;
  }

  return ChannelSPtr();
}