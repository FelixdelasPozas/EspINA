/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// ESPINA
#include "MarchingCubesMesh.h"

#include "Core/Analysis/Data/VolumetricDataUtils.hxx"

using namespace ESPINA;

//----------------------------------------------------------------------------
MarchingCubesMesh::MarchingCubesMesh(Output *output)
: RawMesh(nullptr)
, m_output{output}
, m_lastVolumeModification{VTK_UNSIGNED_LONG_LONG_MAX}
{
}

//----------------------------------------------------------------------------
bool MarchingCubesMesh::isValid() const
{
  return m_output->hasData(VolumetricData<itkVolumeType>::TYPE) || RawMesh::isValid();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> MarchingCubesMesh::mesh() const
{
  auto mesh = RawMesh::mesh();

  if (!mesh || needsUpdate())
  {
    const_cast<MarchingCubesMesh *>(this)->updateMesh();
  }

  return RawMesh::mesh();
}

//----------------------------------------------------------------------------
TimeStamp MarchingCubesMesh::lastModified() const
{
  const_cast<MarchingCubesMesh *>(this)->updateMesh(); // updates the mesh only if necessary.

  return Data::lastModified();
}

//----------------------------------------------------------------------------
void MarchingCubesMesh::updateMesh()
{
  if(!needsUpdate()) return;

  vtkSmartPointer<vtkImageData> image = nullptr;
  TimeStamp volumeTime{VTK_UNSIGNED_LONG_LONG_MAX};

  {
    auto volume = readLockVolume(m_output, DataUpdatePolicy::Ignore);

    if(!volume->isValid()) return;

    image = vtkImage(volume, volume->bounds());
    if(!image)
    {
      qWarning() << "MarchingCubesMesh::updateMesh() -> invalid image for marching cubes.";
      return;
    }

    volumeTime = volume->lastModified();
  }

  int extent[6];
  image->GetExtent(extent);

  --extent[0];
  ++extent[1];
  --extent[2];
  ++extent[3];
  --extent[4];
  ++extent[5];

  // segmentation image need to be padded to avoid segmentation voxels from touching
  // the edges of the image (and create morphologically correct actors)
  auto padding = vtkSmartPointer<vtkImageConstantPad>::New();
  padding->ReleaseDataFlagOn();
  padding->SetInputData(image);
  padding->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
  padding->SetConstant(0);
  padding->UpdateWholeExtent();

  auto marchingCubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
  marchingCubes->ReleaseDataFlagOff();
  marchingCubes->SetNumberOfContours(1);
  marchingCubes->GenerateValues(1, SEG_VOXEL_VALUE, SEG_VOXEL_VALUE);
  marchingCubes->ComputeScalarsOff();
  marchingCubes->ComputeNormalsOff();
  marchingCubes->ComputeGradientsOff();
  marchingCubes->SetInputData(padding->GetOutput());
  marchingCubes->UpdateWholeExtent();

  {
    QMutexLocker lock(&m_lock);

    m_lastVolumeModification = volumeTime;
  }

  setMesh(marchingCubes->GetOutput(), false);
}

//----------------------------------------------------------------------------
QList<Data::Type> MarchingCubesMesh::updateDependencies() const
{
  QList<Data::Type> types;

  types << VolumetricData<itkVolumeType>::TYPE;

  return types;
}

//----------------------------------------------------------------------------
VolumeBounds MarchingCubesMesh::bounds() const
{
  auto bounds = RawMesh::bounds();

  if(!bounds.areValid())
  {
    auto mesh = RawMesh::mesh();

    if (!mesh || needsUpdate())
    {
      const_cast<MarchingCubesMesh *>(this)->updateMesh();
    }
  }

  return RawMesh::bounds();
}

//----------------------------------------------------------------------------
const bool MarchingCubesMesh::needsUpdate() const
{
  TimeStamp time = std::numeric_limits<TimeStamp>::max();

  if(m_output)
  {
    time = readLockVolume(m_output, DataUpdatePolicy::Ignore)->lastModified();
  }

  QMutexLocker lock(&m_lock);

  return m_lastVolumeModification != time;
}
