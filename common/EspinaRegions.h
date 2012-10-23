/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINAREGIONS_H
#define ESPINAREGIONS_H

#include "common/EspinaTypes.h"

/// Volume's voxel's index at given spatial position
EspinaVolume::IndexType VolumeIndex(Nm x, Nm y, Nm z, EspinaVolume *volume);

/// Get the vtk-equivalent extent defining volume
void VolumeExtent(EspinaVolume *volume, int extent[6]);
/// Get the vtk-equivalent bounds defining volume
void VolumeBounds(EspinaVolume *volume, double bounds[6]);

/// Get a normalized region representing for a volume with
/// the given vtk-extent. Normalized regions are used with
/// volumes which origin is set to zero.
EspinaVolume::RegionType ExtentToRegion(int extent[6]);
/// Get a normalized region representing for a volume with
/// the given vtk-bounds. Normalized regions are used with
/// volumes which origin is set to zero.
EspinaVolume::RegionType BoundsToRegion(double bounds[6],
					EspinaVolume::SpacingType spacing);


EspinaVolume::RegionType BoundingBoxRegion(EspinaVolume::RegionType r1,
					   EspinaVolume::RegionType r2);

/// Return a normalized region for volume's largest region
EspinaVolume::RegionType NormalizedRegion(const EspinaVolume *volume);
/// Return the equivalent volume region for a volume's normalized region 
EspinaVolume::RegionType VolumeRegion(EspinaVolume *volume,
                                      EspinaVolume::RegionType normalizedRegion);

#endif // ESPINAREGIONS_H
