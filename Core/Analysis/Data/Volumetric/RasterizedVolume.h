/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_RASTERIZED_VOLUME_H
#define ESPINA_RASTERIZED_VOLUME_H

#include "EspinaCore_Export.h"

// EspINA
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/Mesh/RawMesh.h>

// ITK
#include <itkImageRegionIteratorWithIndex.h>

// VTK
#include <vtkMath.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkImageExport.h>

// Qt
#include <QMutex>

namespace EspINA
{
  template<typename T>
  class EspinaCore_EXPORT RasterizedVolume
  : public SparseVolume<T>
  {
  public:
    /* \brief RasterizedVolume class constructor.
     *
     */
    explicit RasterizedVolume(MeshDataSPtr    mesh,
                              const Bounds    &bounds,
                              const NmVector3 &spacing = NmVector3{1,1,1},
                              const NmVector3 &origin = NmVector3{0,0,0});

    /* \brief RasterizedVolume class virtual destructor.
     *
     */
    virtual ~RasterizedVolume() {};

    /* \brief Implements SparseVolume<T>::memoryUsage().
     *
     */
    virtual size_t memoryUsage() const;

    /* \brief Implements SparseVolume<T>::itkImage().
     *
     */
    virtual const typename T::Pointer itkImage() const;

    /* \brief Implements SparseVolume<T>::itkImage(Bounds).
     *
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    /* \brief Implements SparseVolume<T>::draw(vtkImplicitFunction, Bounds, T::ValueType).
     *
     */
    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /* \brief Implements SparseVolume<T>::draw(BinrayMaskSPtr, T::ValueType).
     *
     */
    virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /* \brief Implements SparseVolume<T>::draw(T::Pointer).
     *
     */
    virtual void draw(const typename T::Pointer volume);

    /* \brief Implements SparseVolume<T>::draw(T::Pointer, Bounds).
     *
     */
    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds);

    /* \brief Implements SparseVolume<T>::draw(T::IndexType, T::PixelType).
     *
     */
    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE);

    /* \brief Implements SparseVolume<T>::fitToContent().
     *
     */
    virtual void fitToContent();

    /* \brief Implements SparseVolume<T>::resize().
     *
     */
    virtual void resize(const Bounds &bounds);

    /* \brief Implements SparseVolume<T>::isEmpty().
     *
     */
    virtual bool isEmpty() const;

  private:
    /* \brief Private method to rasterize a mesh to create an T volume.
     *
     */
    void rasterize() const;

    vtkSmartPointer<vtkPolyData> m_mesh;
    mutable unsigned long int    m_rasterizationTime;
    mutable QMutex               m_mutex;
  };

  template<class T> using RasterizedVolumePtr = RasterizedVolume<T> *;
  template<class T> using RasterizedVolumeSPtr = std::shared_ptr<RasterizedVolume<T>>;

  //----------------------------------------------------------------------------
  template<typename T>
  RasterizedVolume<T>::RasterizedVolume(MeshDataSPtr mesh, const Bounds &meshBounds, const NmVector3 &spacing, const NmVector3 &origin)
  : SparseVolume<T>(meshBounds, spacing, origin)
  , m_mesh             {mesh->mesh()}
  , m_rasterizationTime{0}
  {
  }

  //----------------------------------------------------------------------------
  template<typename T>
  size_t RasterizedVolume<T>::memoryUsage() const
  {
    if(this->m_blocks.empty())
      return 0;

    return SparseVolume<T>::memoryUsage();
  }

  //----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer RasterizedVolume<T>::itkImage() const
  {
    if(this->m_blocks.empty())
      rasterize();

    return SparseVolume<T>::itkImage();
  }

  //----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer RasterizedVolume<T>::itkImage(const Bounds& bounds) const
  {
    if(this->m_blocks.empty())
      rasterize();

    return SparseVolume<T>::itkImage(bounds);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::draw(const vtkImplicitFunction*  brush,
                                 const Bounds&               bounds,
                                 const typename T::ValueType value)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::draw(brush, bounds, value);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                                 const typename T::ValueType value)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::draw(mask, value);
  }


  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::draw(const typename T::Pointer volume)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::draw(volume);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::draw(const typename T::Pointer volume,
                                 const Bounds&             bounds)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::draw(volume, bounds);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::draw(const typename T::IndexType index,
                                 const typename T::PixelType value)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::draw(index, value);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::fitToContent()
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::fitToContent();
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::resize(const Bounds &bounds)
  {
    if(this->m_blocks.empty())
      rasterize();

    SparseVolume<T>::resize(bounds);
  }

  //----------------------------------------------------------------------------
  template<typename T>
  bool RasterizedVolume<T>::isEmpty() const
  {
    if (!this->isValid())
      return true;

    if(this->m_blocks.empty())
      rasterize();

    return SparseVolume<T>::isEmpty();
  }

  //----------------------------------------------------------------------------
  template<typename T>
  void RasterizedVolume<T>::rasterize() const
  {
    this->m_mutex.lock();

    // try to see if already rasterized while waiting in the mutex.
    if (!this->m_blocks.empty() && m_rasterizationTime == m_mesh->GetMTime())
    {
      // already rasterized
      this->m_mutex.unlock();
      return;
    }

    double minSpacing = std::min(this->m_spacing[0], std::min(this->m_spacing[1], this->m_spacing[2]));
    double meshBounds[6];
    double point[3];

    vtkSmartPointer<vtkImplicitPolyDataDistance> distance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    distance->SetInput(m_mesh);
    distance->SetTolerance(0);

    m_mesh->GetBounds(meshBounds);
    auto rasterizationBounds = Bounds{meshBounds[0], meshBounds[1], meshBounds[2], meshBounds[3], meshBounds[4], meshBounds[5]};

    auto region = equivalentRegion<T>(this->m_origin, this->m_spacing, rasterizationBounds);
    typename T::Pointer image = create_itkImage<T>(rasterizationBounds, SEG_BG_VALUE, this->m_spacing, this->m_origin);

    itk::ImageRegionIteratorWithIndex<T> it(image, image->GetLargestPossibleRegion());
    it.GoToBegin();
    while(!it.IsAtEnd())
    {
      auto index = it.GetIndex();

      for(auto i: {0,1,2})
        point[i] = index[i] * this->m_spacing[i];

      if (std::abs(distance->EvaluateFunction(point)) <= minSpacing)
      {
        it.Set(SEG_VOXEL_VALUE);
      }
      else
        it.Set(SEG_BG_VALUE);

      ++it;
    }
    m_rasterizationTime = m_mesh->GetMTime();

    this->setBlock(image, false);
    this->updateModificationTime();

    this->m_mutex.unlock();
  }

} // namespace EspINA
#endif // ESPINA_RASTERIZED_VOLUME_H
