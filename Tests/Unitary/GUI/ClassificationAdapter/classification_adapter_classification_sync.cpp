/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "classification_adapter_testing_support.h"

#include <GUI/Model/ClassificationAdapter.h>

using namespace std;
using namespace ESPINA;

int classification_adapter_classification_sync( int argc, char** argv )
{
  bool error = false;

  ClassificationSPtr classification{new Classification()};
  ClassificationAdapterSPtr adapter{new ClassificationAdapter(classification)};

  QString name1 = "Level 1";
  CategoryAdapterSPtr category1= adapter->createCategory(name1);

  QString name2_1 = "Level 1/Level 2/Level 2-1";
  adapter->createCategory(name2_1);

  QString name2 = "Level 1/Level 2";
  CategoryAdapterSPtr category2 = adapter->category(name2);

  QString name2_2 = "Level 1/Level 2/Level 2-2";
  adapter->createCategory("Level 2-2", category2);

  QString name2_3 = "Level 1/Level 2/Level 2-3";
  CategoryAdapterSPtr category2_3 = adapter->createCategory("Level 2-3", category2);

  error |= TestCategoryAndAdapterSync(adapter->root(), classification->root());

  category2->removeSubCategory(category2_3);

  error |= TestCategoryAndAdapterSync(adapter->root(), classification->root());

  category1->removeSubCategory(category2);

  error |= TestCategoryAndAdapterSync(adapter->root(), classification->root());

  return error;
}


