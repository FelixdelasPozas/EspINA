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
#include "Core/Analysis/Output.h"
#include <Core/Analysis/DataProxy.h>
#include "output_testing_support.h"

using namespace EspINA;
using namespace std;

class InvalidDataProxy;

class InvalidData
: public Data
{
public:
  virtual DataProxySPtr createProxy() const;
  virtual Bounds bounds() const{}
  virtual Snapshot editedRegionsSnapshot() const{}
  virtual bool isValid() const {return false;}
  virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix) {return false; }
  virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const{return Snapshot();}
  virtual Type type() const { return "InvalidData";}
};

using InvalidDataSPtr = std::shared_ptr<InvalidData>;

class InvalidDataProxy
: public DataProxy
{
public:
  virtual DataSPtr get() const
  { return m_data; }
  virtual void set(DataSPtr data)
  { m_data = std::dynamic_pointer_cast<InvalidData>(data); }

private:
  InvalidDataSPtr m_data;
};

DataProxySPtr InvalidData::createProxy() const
{
  return DataProxySPtr{new InvalidDataProxy()};
}

int output_invalid_output( int argc, char** argv )
{
  bool error = false;

  Output output(nullptr, 0);

  if (output.isValid()) {
    cerr << "Default output constructor creates an invalid output" << endl;
    error = true;
  }

  Testing::DummyFilter filter;
  Output output2(&filter, 0);
  
  if (output2.isValid()) {
    cerr << "Output has no associated data" << endl;
    error = true;
  }

  InvalidDataSPtr data{new InvalidData()};

  output2.setData(data);

  if (output2.isValid()) {
    cerr << "There is at least one output data which is invalid. Output cannot be valid" << endl;
    error = true;
  }

  return error;
}