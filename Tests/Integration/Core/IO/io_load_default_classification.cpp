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

#include <Core/IO/ClassificationXML.h>

using namespace std;
using namespace EspINA;
using namespace EspINA::IO;

int io_load_default_classification(int argc, char** argv)
{
  bool error = false;

  QFileInfo input(":/espina/defaultClassification.xml");

  try
  {
    auto defaultClassification = ClassificationXML::load(input);

    auto synapse = defaultClassification->node("Synapse");
    if (!synapse)
    {
      error = true;
      cerr << "Category synapse not found" << endl;
    }

    if (!synapse->properties().contains("Dim_X"))
    {
      error = true;
      cerr << "Synapse: Property Dim X not found" << endl;
    } else if (synapse->property("Dim_X").toUInt() != 500) {
      error = true;
      cerr << "Synapse: Unexpected value at property Dim X" << endl;
    }

    auto vesicle = defaultClassification->node("Vesicle");
    if (!vesicle)
    {
      error = true;
      cerr << "Category vesicle not found" << endl;
    }
    if (!vesicle->properties().contains("Dim_X"))
    {
      error = true;
      cerr << "Vesicle: Property Dim X not found" << endl;
    } else if (vesicle->property("Dim_X").toUInt() != 100) {
      error = true;
      cerr << "Vesicle: Unexpected value at property Dim X" << endl;
    }
  } catch (...) {
    cerr << "Couldn't load default classification" << endl;
    error = true;
  }

  return error;
}