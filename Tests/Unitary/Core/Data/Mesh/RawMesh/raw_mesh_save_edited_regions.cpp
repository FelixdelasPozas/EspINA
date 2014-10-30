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

#include "Core/Analysis/Data/Volumetric/SparseVolume.hxx"
#include "Tests/Testing_Support.h"

#include <vtkSmartPointer.h>

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;

int sparse_volume_save_edited_regions( int argc, char** argv )
{
  bool error = false;

  itkVolumeType::PixelType fg = 75;

  Bounds bounds{-0.5, 3.5, -0.5, 3.5, -0.5, 3.5};
  SparseVolume<itkVolumeType> canvas(bounds);

  if (!canvas.editedRegions().isEmpty()) {
    cerr << "Empty spare volume shouldn't have edited regions " << endl;
    error = true;
  }

  Bounds regions[] = {{-0.5,1.5,-0.5,3.5,-0.5,0.5},
                     {-0.5,3.5,-0.5,1.5,2.5,3.5}};

  canvas.draw(regions[0], fg);

  if (canvas.editedRegions().size() != 1) {
    cerr << "Unxexpected number of edited regions " << endl;
    error = true;
  }

  canvas.draw(regions[1], fg);

  if (canvas.editedRegions().size() != 2) {
    cerr << "Unxexpected number of edited regions " << endl;
    error = true;
  }
  else
  {
    for (int i = 0; i < canvas.editedRegions().size(); ++i) {
      auto editedRegion = canvas.editedRegions().at(i);

      if (editedRegion != regions[i])
      {
        cerr << "Unxexpected edited region " << editedRegion << " differs from " << regions[i] << endl;
        error = true;
      }
    }

    TemporalStorageSPtr storage{new TemporalStorage()};

    auto editedRegionSnapshots = canvas.editedRegionsSnapshot(storage, "sparse", "0");

    if (editedRegionSnapshots.size() != 4)
    {
      cerr << "Unxexpected number of edited regions snapshots" << endl;
      error = true;
    }

    for (int i = 0; i < 2; ++i)
    {
      storage->saveSnapshot(editedRegionSnapshots[2*i  ]);
      storage->saveSnapshot(editedRegionSnapshots[2*i+1]);

      auto filename     = storage->absoluteFilePath(editedRegionSnapshots[2*i].first);
      auto editedVolume = readVolume<itkVolumeType>(filename);
      auto editedBounds = equivalentBounds<itkVolumeType>(editedVolume, editedVolume->GetLargestPossibleRegion());

      if (regions[i] != editedBounds)
      {
        cerr << "Unxexpected edited volume bounds " << editedBounds << " differs from " << regions[i] << endl;
        error = true;
      }

      if (!Testing_Support<itkVolumeType>::Test_Pixel_Values(editedVolume, fg)) {
        cerr << "Unexepected loaded voxel values" << endl;
        error = true;
      }
    }
  }

  return error;
}
