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

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Analysis/Data/MeshData.h"
#include "Core/Analysis/Data/VolumetricData.hxx"
#include "Core/Analysis/Data/VolumetricDataUtils.hxx"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>

namespace ESPINA
{

  template<typename T>
  class EspinaCore_EXPORT MarchingCubesMesh
  : public MeshData
  {
  public:
    /** \brief MarchingCubesMesh class constructor.
     * \param[in] output to obtain the volumetric data from
     *
     * NOTE: this data doesn't updates the output to access its volumetric data
     */
    explicit MarchingCubesMesh(Output *output);

    /** \brief MarchingCubesMesh class virtual destructor.
     *
     */
    virtual ~MarchingCubesMesh();

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override;

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override;

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override {/*TODO*/}

    virtual bool isValid() const override
    { return readLockVolume(m_output, DataUpdatePolicy::Ignore)->isValid(); }

    virtual bool isEmpty() const override
    { return readLockVolume(m_output, DataUpdatePolicy::Ignore)->isEmpty(); }

    virtual Bounds bounds() const override
    { return readLockVolume(m_output, DataUpdatePolicy::Ignore)->bounds(); }

    virtual void setSpacing(const NmVector3& spacing) override;

    virtual NmVector3 spacing() const override;

    virtual size_t memoryUsage() const override;

    virtual vtkSmartPointer<vtkPolyData> mesh() const override;

    virtual void setMesh(vtkSmartPointer<vtkPolyData> mesh) override;

    virtual TimeStamp lastModified() const override;

  private:
    /** \brief Applies marching cubes algorithm to the volumetric data to generate a mesh.
     *
     */
    void updateMesh();

    virtual QList<Data::Type> updateDependencies() const override;

    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override;

    Output *m_output;
    mutable vtkSmartPointer<vtkPolyData> m_mesh;
    TimeStamp m_lastVolumeModification;
  };

  //----------------------------------------------------------------------------
  template <typename T>
  MarchingCubesMesh<T>::MarchingCubesMesh(Output *output)
  : m_output{output}
  , m_mesh  {nullptr}
  , m_lastVolumeModification{VTK_UNSIGNED_LONG_LONG_MAX}
  {
  }

  //----------------------------------------------------------------------------
  template <typename T>
  MarchingCubesMesh<T>::~MarchingCubesMesh()
  {
  }

  //----------------------------------------------------------------------------
  template<typename T>
  bool MarchingCubesMesh<T>::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
  {
    return MeshData::fetchDataImplementation(storage, path, id, bounds);
  }


  //----------------------------------------------------------------------------
  template <typename T>
  Snapshot MarchingCubesMesh<T>::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
  {
    return MeshData::snapshot(storage, path, id);
  }

  //----------------------------------------------------------------------------
  template <typename T>
  Snapshot MarchingCubesMesh<T>::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
  {
    return Snapshot();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::setSpacing(const NmVector3& spacing)
  {
    // NOTE: not allowed
  }

  //----------------------------------------------------------------------------
  template <typename T>
  NmVector3 MarchingCubesMesh<T>::spacing() const
  {
    return m_output->spacing();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  size_t MarchingCubesMesh<T>::memoryUsage() const
  {
    return m_mesh?m_mesh->GetActualMemorySize()*1024:0;
  }

  //----------------------------------------------------------------------------
  template <typename T>
  vtkSmartPointer<vtkPolyData> MarchingCubesMesh<T>::mesh() const
  {
    if (!m_mesh)
    {
      const_cast<MarchingCubesMesh<T> *>(this)->updateMesh();
    }

    return m_mesh;
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::setMesh(vtkSmartPointer< vtkPolyData > mesh)
  {
    m_mesh = mesh;

    BoundsList editedRegions;
    if (m_mesh)
    {
      editedRegions << bounds();
    }
    setEditedRegions(editedRegions);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  TimeStamp MarchingCubesMesh<T>::lastModified() const
  {
    const_cast<MarchingCubesMesh<T> *>(this)->updateMesh(); // updates the mesh only if necessary.
    
    return Data::lastModified();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  void MarchingCubesMesh<T>::updateMesh()
  {
    auto volume = readLockVolume(m_output, DataUpdatePolicy::Ignore);

    if(m_lastVolumeModification == volume->lastModified()) return;

    auto image = vtkImage(volume, volume->bounds());

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
    padding->SetInputData(image);
    padding->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
    padding->SetConstant(0);
    padding->UpdateWholeExtent();

    auto marchingCubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();

    marchingCubes->ReleaseDataFlagOn();
    marchingCubes->SetNumberOfContours(1);
    marchingCubes->GenerateValues(1, SEG_VOXEL_VALUE, SEG_VOXEL_VALUE);
    marchingCubes->ComputeScalarsOff();
    marchingCubes->ComputeNormalsOff();
    marchingCubes->ComputeGradientsOff();
    marchingCubes->SetInputData(padding->GetOutput());
    marchingCubes->Update();

    if(!m_mesh)
    {
      m_mesh = vtkSmartPointer<vtkPolyData>::New();
    }

    m_mesh->DeepCopy(marchingCubes->GetOutput());

    m_lastVolumeModification = volume->lastModified();

    updateModificationTime();
  }

  //----------------------------------------------------------------------------
  template <typename T>
  QList<Data::Type> MarchingCubesMesh<T>::updateDependencies() const
  {
    QList<Data::Type> types;

    types << VolumetricData<itkVolumeType>::TYPE;

    return types;
  }

} // namespace ESPINA

#endif // ESPINA_MARCHING_CUBES_MESH_H
