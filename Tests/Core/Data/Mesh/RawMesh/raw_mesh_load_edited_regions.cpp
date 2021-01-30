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

#include "Tests/Testing_Support.h"

#include <Core/Analysis/Data/Mesh/RawMesh.h>
#include <Core/Utils/vtkPolyDataUtils.h>

#include "MeshTestingUtils.h"

using namespace std;
using namespace ESPINA;
using namespace ESPINA::Testing;
using namespace ESPINA::PolyDataUtils;

int raw_mesh_load_edited_regions(int argc, char** argv)
{
  bool error = false;

  Bounds bounds{ -0.5, 3.5, -0.5, 3.5, -0.5, 3.5 };

  RawMesh rawMesh;
  if (!rawMesh.editedRegions().isEmpty())
  {
    cerr << "Empty raw mesh shouldn't have edited regions " << endl;
    error = true;
  }

  auto mesh = createTriangleMesh();

  rawMesh.setMesh(mesh);

  if ((rawMesh.mesh()->GetNumberOfCells() != mesh->GetNumberOfCells()) ||
      (rawMesh.mesh()->GetNumberOfLines() != mesh->GetNumberOfLines()) ||
      (rawMesh.mesh()->GetNumberOfPoints() != mesh->GetNumberOfPoints()) ||
      (rawMesh.mesh()->GetNumberOfPolys() != mesh->GetNumberOfPolys()) ||
      (rawMesh.mesh()->GetNumberOfVerts() != mesh->GetNumberOfVerts()))
  {
    cerr << "Unexpected mesh polydata" << endl;
    error = true;
  }

  if (rawMesh.editedRegions().size() != 0)
  {
    cerr << "Unexpected number of edited regions" << endl;
    error = true;
  }
  else
  {
    rawMesh.setMesh(mesh);

    if (rawMesh.editedRegions().size() != 1)
    {
      cerr << "Unexpected number of edited regions" << endl;
      error = true;
    }

    auto editedRegions = rawMesh.editedRegions();
    auto editedRegion = editedRegions.first();

    if (editedRegion != rawMesh.bounds())
    {
      cerr << "Unxexpected edited region " << editedRegion << " differs from " << rawMesh.bounds() << endl;
      error = true;
    }
    auto storage = std::make_shared<TemporalStorage>();

    auto editedRegionSnapshots = rawMesh.editedRegionsSnapshot(storage, "mesh", "0");

    if (editedRegionSnapshots.size() != 1)
    {
      cerr << "Unxexpected number of edited regions snapshots" << endl;
      error = true;
    }
    else
    {
      auto snapshot = editedRegionSnapshots.first();
      storage->saveSnapshot(snapshot);
    }

    rawMesh.setMesh(nullptr); // Clear mesh
    rawMesh.setEditedRegions(editedRegions); // restore expected edited regions

    if (rawMesh.mesh() == mesh)
    {
      cerr << "Unexpected mesh polydata" << endl;
      error = true;
    }

    rawMesh.restoreEditedRegions(storage, "mesh", "0");

    if (editedRegions != rawMesh.editedRegions())
    {
      cerr << "Edited regions shouldn't be modified after restoration" << endl;
      error = true;
    }

    auto restoredMesh = rawMesh.mesh();

    if (restoredMesh->GetNumberOfPoints() != mesh->GetNumberOfPoints())
    {
      cerr << "Unxexpected number of edited region points" << endl;
      error = true;
    }

    if (restoredMesh->GetNumberOfCells() != mesh->GetNumberOfCells())
    {
      cerr << "Unxexpected number of edited region cells" << endl;
      error = true;
    }

  }

  return error;
}
