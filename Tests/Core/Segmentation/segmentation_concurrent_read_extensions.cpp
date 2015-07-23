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

int segmentation_concurrent_read_extensions( int argc, char** argv )
{
  bool error = false;

  auto filter       = std::make_shared<Testing::DummyFilter>();
  auto segmentation = std::make_shared<Segmentation>(getInput(filter, 0));
  auto extension    = std::make_shared<DummySegmentationExtension>();

  {
    auto extensions = segmentation->extensions();
    extensions->add(extension);
  }

  vector<thread> threads;
  int numTasks         = 2;
  int numFinishedTasks = 0;

  for(int i = 0; i < numTasks; ++i)
  {
    threads.push_back(thread([&, i](){
      auto extensions = segmentation->readOnlyExtensions();

      if (!extensions->hasExtension(extension->type())) {
        cerr << "Unxpected output data bounds" << endl;
        error = true;
      }

      if (i == 0)
      {
        usleep(1000);
        numFinishedTasks++;
      }
      else if (numFinishedTasks == 1)
      {
        cerr << "Unxpected finalization order" << endl;
        error = true;
      }

      cout << "Task " << i << " finished" << endl;
    }));
  }

  for(auto& thread : threads){
    thread.join();
  }


  return error;
}