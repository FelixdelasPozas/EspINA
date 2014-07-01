/*
 
 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// EspINA
#ifndef SEGMENTATIONCOLLISION_H
#define SEGMENTATIONCOLLISION_H

#include "EspinaCore_Export.h"

#include <Core/OutputRepresentations/RawVolume.h>

namespace EspINA
{
  // checks if both volumes collide at the voxel level
  bool checkCollision(SegmentationVolumeSPtr seg1,
                      SegmentationVolumeSPtr seg2);
}

#endif // SEGMENTATIONCOLLISION_H
