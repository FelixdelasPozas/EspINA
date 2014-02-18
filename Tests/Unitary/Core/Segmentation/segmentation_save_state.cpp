/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Analysis.h"
#include "Core/Analysis/Category.h"
#include <Core/MultiTasking/Scheduler.h>
#include "segmentation_testing_support.h"

#include <iostream>
#include <QDebug>

using namespace EspINA;
using namespace EspINA::Testing;

int segmentation_save_state(int argc, char** argv)
{
  SegmentationSPtr segmentation{new Segmentation(InputSPtr())};

  State forgedState;

  segmentation->setNumber(1);
  forgedState = "NUMBER=1;";

  segmentation->modifiedByUser("AUser");
  forgedState += "USERS=AUser;";

  Classification classification;
  classification.createNode("Prueba", classification.root());

  segmentation->setCategory(classification.node("Prueba"));
  forgedState += "CATEGORY=Prueba;";

  segmentation->setAlias("HPD4");
  forgedState += "ALIAS=HPD4;";

  return (segmentation->state() != forgedState);
}
