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
#include "testing_support_dummy_filter.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int output_replace_data( int argc, char** argv )
{
  static VolumeBounds modified(Bounds{1,2,3,4,5,6});

  class NoProxyData
  : public DummyData
  {
    public:
      virtual VolumeBounds bounds() const
      { return modified; }

      virtual DataSPtr createProxy() const { throw -1; }
  };

  bool error = false;

  DummyFilter filter;

  Output output(&filter, 0, NmVector3{1,1,1});

  auto data = std::make_shared<DummyData>();
  output.setData(data);

  if (output.readLockData<Data>(data->type())->bounds() == modified)
  {
    cerr << "Unxpected data bounds" << endl;
    error = true;
  }

  auto noProxyData = std::make_shared<NoProxyData>();
  try
  {
    output.setData(noProxyData);
  }
  catch (...)
  {
    cerr << "Output is creating a new data proxy instead of replacing proxy delegate" << endl;
    error = true;
  }

  if (output.readLockData<Data>(data->type())->bounds() != modified)
  {
    cerr << "Unxpected data bounds" << endl;
    error = true;
  }

  return error;
}
