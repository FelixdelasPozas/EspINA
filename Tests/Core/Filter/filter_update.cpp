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
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/MultiTasking/Scheduler.h>
#include "filter_testing_support.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;


int filter_update( int argc, char** argv )
{
  bool error = false;

  DummyFilter *filter{new DummyFilter()};

  filter->update();

  auto output = filter->output(0);

  if (output->isEdited()) {
    cerr << "Unexpected filter output modifications" <<  endl;
    error = true;
  }

  auto volume = writeLockVolume(output);
  itkVolumeType::IndexType index;
  index.Fill(0);
  volume->draw(index);

  if (!output->isEdited()) {
    cerr << "Exepected filter output modifications" <<  endl;
    error = true;
  }

  filter->update();

  if (output->isEdited()) {
    cerr << "Unexpected filter output modifications after re-execution" <<  endl;
    error = true;
  }
  return error;
}