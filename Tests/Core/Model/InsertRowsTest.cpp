/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#include "InsertRowsTest.h"

#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/EspinaTypes.h>
#include <Filters/FreeFormSource.h>

void insertRowsTest(EspINA::EspinaModel* model)
{
//   Filter::NamedInputs inputs;
//   Filter::Arguments args;
// 
//   FreeFormSource::Parameters params(args);
//   double spacing[3] = {1, 1, 1};
//   params.setSpacing(spacing);
// 
//   FreeFormSource source(inputs, args);
//   source.draw(0, 50., 50., 50.);
//   source.update();

  EspINA::Filter::OutputId outputId = -1;
  EspINA::FilterSPtr filterp = EspINA::FilterSPtr(NULL);
  EspINA::SegmentationPtr p1 = new EspINA::Segmentation(filterp, outputId);
  EspINA::SegmentationSPtr sharedp1 = EspINA::SegmentationSPtr(p1);

  model->addSegmentation(sharedp1);
}
