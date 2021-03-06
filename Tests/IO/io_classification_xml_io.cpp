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
using namespace ESPINA;
using namespace ESPINA::IO;

int io_classification_xml_io(int argc, char** argv)
{
  bool error = false;

  ClassificationSPtr classification{new Classification("test")};

  classification->createNode("1");
  classification->createNode("1/1");
  classification->createNode("1/2");
  classification->createNode("2");
  classification->createNode("3");
  classification->createNode("3/1/1");

  QFileInfo file("classification.xml");
  try
  {
    ClassificationXML::save(classification, file);
  } catch (...) {
    cerr << "Couldn't save classification" << endl;
    error = true;
  }

  ClassificationSPtr loadedClassification;
  try
  {
    loadedClassification = ClassificationXML::load(file);
  } catch (...) {
    cerr << "Couldn't load classification" << endl;
    error = true;
  }

  if (print(classification) != print(loadedClassification)) {
    cerr << "Unexpected Loaded Classification" << endl;
    cerr << print(classification).toStdString();
    cerr << print(loadedClassification).toStdString();
    error = true;
  }

  return error;
}