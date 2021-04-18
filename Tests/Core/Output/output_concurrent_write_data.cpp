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

#include <thread>
#include <iostream>
#include <vector>
#include <atomic>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int output_concurrent_write_data( int argc, char** argv )
{
  std::atomic<bool> error{false};

  DummyFilter filter;

  Output output(&filter, 0, NmVector3{1,1,1});

  auto data = make_shared<DummyData>();

  output.setData(data);

  if (!output.isValid())
  {
    cerr << "Output is not initialized with a valid filter and a valid output" << endl;
    error = true;
  }

  vector<thread> threads;

  threads.push_back(thread([&error, &output, data](){
    cout << "Write task started and asks for data..." << endl;
    auto writeData = output.writeLockData<Data>(data->type());

    cout << "Write task holds data, reads and waits..." << endl;
    if (writeData->bounds().spacing() != data->spacing()) {
      cerr << "Unxpected output data spacing" << endl;
      error = true;
    }

    usleep(10000);

    writeData->setSpacing({2, 2, 2});

    if (writeData->bounds().spacing() != NmVector3{2, 2, 2}) {
      cerr << "Unxpected output data spacing " << writeData->bounds().spacing() << endl;
      error = true;
    }

    cout << "Write task finishes and releases data" << endl;
  }));

  usleep(100); // give time to write task to hold data.

  for(int i = 0; i < 5; ++i)
  {
  threads.push_back(thread([&error, &output, data, i](){
    cout << "Read thread " << i << " started and waits..." << endl;
    usleep(i * 1000);
    cout << "Read thread " << i << " asks for data..." << endl;
    auto readData = output.readLockData<Data>(data->type());

    cout << "Read task " << i << " has data and reads..." << endl;

    if (readData->bounds().spacing() != NmVector3{2, 2, 2}) {
      cerr << "Unxpected output data spacing " << readData->bounds().spacing() << endl;
      error = true;
    }

    cout << "Read task " << i << " finishes" << endl;
  }));
  }

  for(auto& thread : threads)
  {
    thread.join();
  }

  return error;
}
