/*

    Copyright (C) 2014
    Jorge Pe�a Pastor<jpena@cesvima.upm.es>,
    Felix de las Pozas<fpozas@cesvima.upm.es>

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

// ESPINA
#include "ModelUtils.h"

#include <Core/Analysis/Segmentation.h>

// Qt
#include <QString>

using namespace ESPINA;

//------------------------------------------------------------------------
unsigned int ESPINA::GUI::Model::Utils::firstUnusedSegmentationNumber(const ModelAdapterSPtr model)
{
  unsigned int number = 0;

  for (auto segmentation: model->segmentations())
  {
    if (segmentation->number() > number)
    {
      number = segmentation->number();
    }
  }

  return ++number;
}

