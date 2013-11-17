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

using namespace EspINA;

//-----------------------------------------------------------------------------
EspINA::AnalysisSPtr EspINA::merge(AnalysisSPtr lhs, AnalysisSPtr rhs)
{
  AnalysisSPtr mergedAnalysis{new Analysis()};
  
  if (lhs->classification() && !rhs->classification())
  { // Use lhs classification

  } else if (!lhs->classification() && rhs->classification())
  { // Use rhs classification

  } else if (lhs->classification() && rhs->classification())
  { // Create merged classification

  }

  QMap<SampleSPtr, SampleSPtr> samples;

  for(auto analysis : {lhs, rhs}) 
  {
    for(auto sample : analysis->samples())
    {
      if (!findSample(sample, mergedAnalysis->samples()))
      {
        SampleSPtr mergedSample{new Sample(sample->name())};
        mergedAnalysis->add(mergedSample);
      }
    }

    //TODO: BUG: Errors may result if two analysis share the same objects, thus
    // we need to copy them
    for(auto channel : analysis->channels())
    {
      mergedAnalysis->add(channel);
    }

    for(auto segmentation : analysis->segmentations())
    {
      mergedAnalysis->add(segmentation);
    }
  }

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
