/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Analysis.h"

#include "SegmentationExtensionSupport.h"
#include "segmentation_testing_support.h"

#include <thread>
#include <iostream>
#include <vector>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int segmentation_concurrent_write_extensions( int argc, char** argv )
{
  bool error = false;

  auto filter       = std::make_shared<Testing::DummyFilter>();
  auto segmentation = std::make_shared<Segmentation>(getInput(filter, 0));
  auto extension    = std::make_shared<DummySegmentationExtension>();

  vector<thread> threads;

  threads.push_back(thread([&](){
    auto extensions = segmentation->extensions();

    cout << "Write Task started" << endl;

    if (extensions->hasExtension(extension)) {
      cerr << "Unexpected extension" << endl;
      error = true;
    }

    usleep(1000);

    extensions->add(extension);

    cout << "Write Task finished" << endl;
  }));

  usleep(100);

  threads.push_back(thread([&](){
    auto extensions = segmentation->readOnlyExtensions();

    cout << "Read Task started" << endl;

    if (!extensions->hasExtension(extension)) {
      cerr << "Expected extension not found" << endl;
      error = true;
    }

    cout << "Read Task finished" << endl;
  }));


  for(auto& thread : threads){
    thread.join();
  }


  return error;
}