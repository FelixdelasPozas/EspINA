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

#include "classification_adapter_testing_support.h"

#include <GUI/Model/ClassificationAdapter.h>

using namespace std;
using namespace ESPINA;

int classification_adapter_create_category( int argc, char** argv )
{
  bool error = false;

  ClassificationAdapter classification;
  
  QString name1 = "Level 1";
  CategoryAdapterSPtr category1= classification.createCategory(name1);
  
  error |= TestClassificationName(category1, name1);

  QString name2_1 = "Level 1/Level 2/Level 2-1";
  CategoryAdapterSPtr category3_1 = classification.createCategory(name2_1);
  
  error |= TestClassificationName(category3_1, name2_1);
  
  QString name2 = "Level 1/Level 2";
  CategoryAdapterSPtr category2 = classification.category(name2);
  
  error |= TestClassificationName(category2, name2);
  
  QString name2_2 = "Level 1/Level 2/Level 2-2";
  CategoryAdapterSPtr category3_2 = classification.createCategory("Level 2-2", category2);
  
  error |= TestClassificationName(category3_2, name2_2);
  
  if (classification.parent(category2) != category1) {
    cerr << category2->name().toStdString() << " parent is not " << category1->name().toStdString() << endl;
    error = true;
  }

  if (classification.parent(category3_1) != category2) {
    cerr << category3_1->name().toStdString() << " parent is not " << category2->name().toStdString() << endl;
    error = true;
  }

  if (classification.parent(category3_2) != category2) {
    cerr << category3_2->name().toStdString() << " parent is not " << category2->name().toStdString() << endl;
    error = true;
  }

  return error;
}