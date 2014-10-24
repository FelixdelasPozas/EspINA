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

#include "Filters/SeedGrowSegmentationFilter.h"

#include <testing_support_channel_input.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int seed_grow_segmentation_change_parameters(int argc, char** argv)
{
  bool error = false;

  InputSList   inputs;
  inputs << channelInput();

  Filter::Type  type{"SGS"};

  SchedulerSPtr scheduler;

  SeedGrowSegmentationFilter sgsf(inputs, type, scheduler);

  NmVector3 seed{1, 2, 3};
  int lth = 10;
  int uth = 20;
  int cr  = 5;

  sgsf.setSeed(seed);
  sgsf.setLowerThreshold(lth);
  sgsf.setUpperThreshold(uth);
  sgsf.setClosingRadius(cr);

  if (sgsf.seed() != seed){
    cerr << "Wrong seed value " << endl;
    error = true;
  }

  if (sgsf.lowerThreshold() != lth){
    cerr << "Wrong lower threshold value " << endl;
    error = true;
  }

  if (sgsf.upperThreshold() != uth){
    cerr << "Wrong upper threshold value " << endl;
    error = true;
  }

  if (sgsf.closingRadius() != cr){
    cerr << "Wrong closing radius value " << endl;
    error = true;
  }

  return error;
}