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


#ifndef ESPINA_MARCHING_CUBES_MESH_H
#define ESPINA_MARCHING_CUBES_MESH_H

#include "EspinaCore_Export.h"

#include "Core/Analysis/Data/MeshData.h"
#include "Core/Analysis/Data/VolumetricData.h"
#include "Core/Analysis/Data/VolumetricDataUtils.h"

#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>

namespace EspINA
{

  template<typename T>
  class EspinaCore_EXPORT MarchingCubesMesh
  : public MeshData
  {
  public:
    explicit MarchingCubesMesh(VolumetricDataSPtr<T> volume);

    virtual ~MarchingCubesMesh();

    virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix);

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const;

    virtual Snapshot editedRegionsSnapshot() const;

    virtual bool isValid() const
    { return m_volume->isValid(); }

    virtual Bounds bounds() const
    { return m_volume->bounds(); }

    virtual void setSpacing(const NmVector3& spacing);

    virtual NmVector3 spacing() const;

    virtual void undo();

    virtual size_t memoryUsage() const;

    virtual vtkSmartPointer<vtkPolyData> mesh() const;

//   private slots://TODO
    void updateMesh();

  private:
    VolumetricDataSPtr<T> m_volume;

    vtkSmartPointer<vtkPolyData> m_mesh;
  };

  //----------------------------------------------------------------------------
  template <typename T>
  MarchingCubesMesh<T>::MarchingCubesMesh(VolumetricDataSPtr<T> volume)
  : m_volume(volume)
  {
//     connect(m_volume.get(), SIGNAL(dataChanged()),
// 	    this, SLOT(updateMesh()));
  }

  //----------------------------------------------------------------------------
  template <typename T>
  MarchingCubesMesh<T>::~MarchingCubesMesh()
  {
//     disconnect(m_volume.get(), SIGNAL(dataChanged()),
// 	       this, SLOT(updateMesh()));
  }

  //----------------------------------------------------------------------------
  template <typename T>
  bool MarchingCubesMesh<T>::fetchData(const TemporalStorageSPtr storage, const QString& prefix)
  {
    return false;
  }

  //----------------------------------------------------------------------------
  template <typename T>
  Snapshot MarchingCubesMesh<T>::snapshot(TemporalStorageSPtr storage, const QString& prefix) const
  {
    return Snapshot();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  Snapshot MarchingCubesMesh<T>::editedRegionsSnapshot() const
  {
    return Snapshot();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::setSpacing(const NmVector3& spacing)
  {

  }

  //----------------------------------------------------------------------------
  template <typename T>
  NmVector3 MarchingCubesMesh<T>::spacing() const
  {
    return m_volume->spacing();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::undo()
  {

  }

  //----------------------------------------------------------------------------
  template <typename T>
  size_t MarchingCubesMesh<T>::memoryUsage() const
  {
    return m_mesh->GetActualMemorySize()/1024;
  }

  //----------------------------------------------------------------------------
  template <typename T>
  vtkSmartPointer<vtkPolyData> MarchingCubesMesh<T>::mesh() const
  {
    if (!m_mesh)
      updateMesh();

    return m_mesh;
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::updateMesh()
  {
    vtkImageData *image;//TODO = vtkImage(m_volume, m_volume->bounds());

    int extent[6];
    image->GetExtent(extent);

    extent[0]--;
    extent[1]++;
    extent[2]--;
    extent[3]++;
    extent[4]--;
    extent[5]++;

    // segmentation image need to be padded to avoid segmentation voxels from touching
    // the edges of the image (and create morphologically correct actors)
    auto padding = vtkSmartPointer<vtkImageConstantPad>::New();

    padding->SetInputData(image);
    padding->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
    padding->SetConstant(0);
    padding->UpdateWholeExtent();

    auto marchingCubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();

    marchingCubes->ReleaseDataFlagOn();
    marchingCubes->SetNumberOfContours(1);
    marchingCubes->GenerateValues(1, 255, 255);
    marchingCubes->ComputeScalarsOff();
    marchingCubes->ComputeNormalsOff();
    marchingCubes->ComputeGradientsOff();
    marchingCubes->SetInputData(padding->GetOutput());

    marchingCubes->Update();

    m_mesh->DeepCopy(marchingCubes->GetOutput());
  }



} // namespace EspINA

#endif // ESPINA_MARCHING_CUBES_MESH_H