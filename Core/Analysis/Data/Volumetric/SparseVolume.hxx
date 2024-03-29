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

#ifndef ESPINA_SPARSE_VOLUME_H
#define ESPINA_SPARSE_VOLUME_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/Vector3.hxx>

// ITK
#include <itkImageRegionIterator.h>
#include <itkImageRegionExclusionIteratorWithIndex.h>

// VTK
#include <vtkImplicitFunction.h>

// Qt
#include <QMap>
#include <QReadWriteLock>

namespace ESPINA
{
  /*! \brief Volume representation intended to save memory and speed up
   *  edition operations
   *
   *  Voxels which don't belong to any block are assigned the value
   *  defined as background value.
   *
   */
  template<typename T>
  class EspinaCore_EXPORT SparseVolume
  : public VolumetricData<T>
  {
    public:
      /** \brief SparseVolume class constructor
       * \param[in] bounds bounds of the empty volume.
       * \param[in] spacing spacing of the volume.
       * \param[in] origin origin of the volume.
       *
       */
      explicit SparseVolume(const Bounds& bounds = Bounds(), const NmVector3& spacing = {1, 1, 1}, const NmVector3& origin = NmVector3());

      explicit SparseVolume(const VolumeBounds& bounds);

      /** \brief SparseVolume class constructor
       * \param[in] volume initial content of the sparse volume.
       * \param[in] bounds bounds of the empty volume.
       * \param[in] spacing spacing of the volume.
       * \param[in] origin origin of the volume.
       *
       */
      explicit SparseVolume(const typename T::Pointer volume, const Bounds& bounds, const NmVector3& spacing = {1, 1, 1}, const NmVector3& origin = NmVector3());

      /** \brief SparseVolume class virtual destructor.
       *
       */
      virtual ~SparseVolume()
      {}

      /** \brief Returns the memory usage in bytes of the volume.
       *
       */
      virtual size_t memoryUsage() const override;

      virtual void setOrigin(const NmVector3& origin) override;

      virtual void setSpacing(const NmVector3& spacing) override;

      virtual const typename T::Pointer itkImage() const override;

      virtual const typename T::Pointer itkImage(const Bounds& bounds) const override;

      virtual void draw(vtkImplicitFunction        *brush,
                        const Bounds&               bounds,
                        const typename T::ValueType value = SEG_VOXEL_VALUE) override;

      virtual void draw(const typename T::Pointer image)                    override;

      virtual void draw(const typename T::Pointer image,
                        const Bounds&             bounds)                    override;

      virtual void draw(const typename T::IndexType &index,
                        const typename T::ValueType value = SEG_VOXEL_VALUE) override;

      virtual void draw(const Bounds               &bounds,
                        const typename T::ValueType value = SEG_VOXEL_VALUE) override;

      virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                        const typename T::ValueType value = SEG_VOXEL_VALUE) override;


      virtual void resize(const Bounds &bounds) override;

      virtual const typename T::RegionType itkRegion() const override final;

      virtual const typename T::SpacingType itkSpacing() const override final;

      virtual const typename T::PointType itkOriginalOrigin() const override final;

      virtual bool isValid() const override;

      virtual bool isEmpty() const override;

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override;

      virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) override;

      virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override;

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override;

    private:
      using BlockIndexes = QList<lliVector3>;

      QString editedRegionSnapshotId(const QString &outputId, const int regionId) const
      { return QString("%1_%2_EditedRegion_%3.mhd").arg(outputId).arg(this->type()).arg(regionId);}

      /** \brief Returns the list of block indexes that comprises (total or partially)
       * the region passed as the argument.
       * \param[in] bounds bounds.
       *
       */
      BlockIndexes toBlockIndexes(const itk::ImageRegion<3> &region) const;

      /** \brief Returns the list of block indexes that comprises (total or partially)
       * the bounds passed as the argument.
       * \param[in] bounds bounds.
       *
       */
      BlockIndexes toBlockIndexes(const Bounds &bounds) const;

      /** \brief Update the sparse volume time stamp and edited regions and return the affected block indexes
       *
       */
      Bounds editRegion(const ESPINA::VolumeBounds &bounds);

      /** \brief Creates a block for the given index
       * \param[in] index block index.
       *
       */
      void createBlock(const lliVector3 &index);

      /** \brief Returns true if the block associated with the given index is empty.
       * \param[in] index block index.
       *
       */
      bool isEmpty(const lliVector3 &index) const;

      /** \brief Returns an iterator to iterate all pixels of block containted in bounds
       * \param[in] block index
       * \param[in] bounds to iterate its voxels
       *
       */
      Bounds blockIntersection(typename T::Pointer block, const Bounds &bounds) const;

      /** \brief Helper method to assist fetching data from disk.
       *
       */
      QString singleBlockPath(const QString &id) const
      { return QString("%1_%2.mhd").arg(id).arg(this->type()); }

      /** \brief Helper method to assist fetching data from disk.
       *
       */
      QString multiBlockPath(const QString &id, int part) const
      { return QString("%1_%2_%3.mhd").arg(id).arg(this->type()).arg(part); }

      /** \brief Helper method to assist fetching data from disk.
       *
       */
      QString oldSingleBlockPath(const QString &id) const
      { return QString("%1_%2.mhd").arg(this->type()).arg(id); }

      /** \brief Helper method to assist fetching data from disk.
       *
       */
      QString oldMultiBlockPath(const QString &id, int part) const
      { return QString("%1_%2_%3.mhd").arg(this->type()).arg(id).arg(part); }

      virtual QList<Data::Type> updateDependencies() const override
      { return QList<Data::Type>(); }

      /** \brief Helper method to generate a snapshot for a image block.
       * \param[in] volume Block image.
       * \param[in] storage Temporal storage.
       * \param[in] path Snapshot path.
       * \param[in] id Image id (filename in storage).
       *
       */
      Snapshot blockSnapshotMemory(typename T::Pointer volume, TemporalStorageSPtr storage, const QString &path, const QString &id);

    protected:
      const unsigned int s_blockSize = 25;
      QMap<lliVector3, typename T::Pointer> m_blocks;

      mutable QReadWriteLock m_blockMutex;
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  {
    this->m_bounds = VolumeBounds(bounds, spacing, origin);
    this->setBackgroundValue(0);
  }
  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const VolumeBounds& bounds)
  : SparseVolume<T>(bounds, bounds.spacing(), bounds.origin())
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const typename T::Pointer volume, const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : SparseVolume<T>(bounds, spacing, origin)
  {
    draw(volume, bounds);

    this->clearEditedRegions();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  size_t SparseVolume<T>::memoryUsage() const
  {
    return std::pow(s_blockSize, 3) * m_blocks.size();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setOrigin(const NmVector3& origin)
  {
    //NOTE: 2015-04-20 Review when tiling support added
    //NmVector3 shift = m_origin - origin;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setSpacing(const NmVector3& spacing)
  {
    auto origin      = this->m_bounds.origin();
    auto prevSpacing = this->m_bounds.spacing();

    if (prevSpacing != spacing)
    {
      auto itkSpacing   = ItkSpacing<T>(spacing);
      auto spacingRatio = spacing/prevSpacing;

      for(auto &block : m_blocks)
      {
        changeSpacing<T>(block, itkSpacing, spacingRatio);
      }

      BoundsList regions;
      for (auto regionBounds : this->editedRegions())
      {
        VolumeBounds bounds(regionBounds, prevSpacing, origin);

        regions << ESPINA::changeSpacing(bounds, spacing);
      }
      this->setEditedRegions(regions);

      this->m_bounds = ESPINA::changeSpacing(this->m_bounds, spacing);
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage() const
  {
    return itkImage(this->m_bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage(const Bounds& bounds) const
  {
    if (!bounds.areValid())
    {
      auto what = QObject::tr("Invalid input bounds");
      auto details = QObject::tr("SparseVolume::itkImage(bounds) -> Invalid input bounds.");

      throw Core::Utils::EspinaException(what, details);
    }

    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds expectedBounds(bounds, spacing, origin);

    constexpr unsigned long long MEGA = 1024*1024;
    auto image = create_itkImage<T>(expectedBounds.bounds(), this->backgroundValue(), spacing, origin);

    if(!intersect(this->m_bounds, expectedBounds))
    {
      return image;
// @felix disabled 2022 due to strange SEG files with 2 tiff with different spacing.
//      auto what = QObject::tr("Invalid input bounds, no intersection");
//      auto details = QObject::tr("SparseVolume::itkImage(bounds) -> Invalid input bounds, input: %1, volume bounds: %2").arg(expectedBounds.toString()).arg(this->m_bounds.toString());
//
//      throw Core::Utils::EspinaException(what, details);
    }

    m_blockMutex.lockForRead();

    auto affectedIndexes = toBlockIndexes(expectedBounds.bounds());

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index)) continue;

      auto block        = m_blocks[index];
      auto commonBounds = blockIntersection(block, expectedBounds);

      copy_image<itkVolumeType>(block, image, commonBounds);
    }

    m_blockMutex.unlock();

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(vtkImplicitFunction        *brush,
                             const Bounds               &bounds,
                             const typename T::ValueType value)
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds requestedBounds(bounds, spacing, origin);

    auto editedBounds    = editRegion(requestedBounds);
    auto affectedIndexes = toBlockIndexes(editedBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        if(value == this->backgroundValue())
        {
          continue;
        }

        createBlock(index);
      }

      auto block       = m_blocks[index];
      auto blockBounds = blockIntersection(block, editedBounds);

      auto bit = itkImageIteratorWithIndex<T>(block, blockBounds);
      while(!bit.IsAtEnd())
      {
        auto index = bit.GetIndex();
        NmVector3 point{index[0]*spacing[0] + spacing[0]/2,
                        index[1]*spacing[1] + spacing[1]/2,
                        index[2]*spacing[2] + spacing[2]/2};

        if (brush->FunctionValue(point[0], point[1], point[2]) <= 0)
        {
          bit.Set(value);
        }

        ++bit;
      }

      if(value == this->backgroundValue() && isEmpty(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds requestedBounds(mask->bounds(), spacing, origin);

    auto editedBounds    = editRegion(requestedBounds);
    auto affectedIndexes = toBlockIndexes(editedBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        if(value == this->backgroundValue())
        {
          continue;
        }

        createBlock(index);
      }

      auto block       = m_blocks[index];
      auto blockBounds = blockIntersection(block, editedBounds);

      auto bit = itkImageIterator<T>(block, blockBounds);
      BinaryMask<unsigned char>::region_iterator mit(mask.get(), blockBounds);

      mit.goToBegin();
      while(!mit.isAtEnd())
      {
        if(mit.isSet())
        {
          bit.Set(value);
        }

        ++mit;
        ++bit;
      }

      if(value == this->backgroundValue() && isEmpty(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer image)
  {
    Bounds bounds = equivalentBounds<T>(image);

    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds requestedBounds(bounds, spacing, origin);

    auto editedBounds    = editRegion(requestedBounds);
    auto affectedIndexes = toBlockIndexes(editedBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        createBlock(index);
      }

      auto block        = m_blocks[index];
      auto commonBounds = blockIntersection(block, editedBounds);

      copy_image<itkVolumeType>(image, block, commonBounds);

      if(isEmpty(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer image,
                             const Bounds&             bounds)
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds requestedBounds(bounds, spacing, origin);

    auto editedBounds    = editRegion(requestedBounds);
    auto affectedIndexes = toBlockIndexes(editedBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        createBlock(index);
      }

      auto block        = m_blocks[index];
      auto commonBounds = blockIntersection(block, editedBounds);

      copy_image<itkVolumeType>(image, block, commonBounds);

      if(isEmpty(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const typename T::IndexType &index,
                             const typename T::ValueType  value)
  {
    auto spacing = this->m_bounds.spacing();

    Bounds bounds { index[0] * spacing[0], index[0] * spacing[0],
                    index[1] * spacing[1], index[1] * spacing[1],
                    index[2] * spacing[2], index[2] * spacing[2]};

    draw(bounds, value);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const Bounds               &bounds,
                             const typename T::ValueType value)
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    VolumeBounds requestedBounds(bounds, spacing, origin);

    auto editedBounds    = editRegion(requestedBounds);
    auto affectedIndexes = toBlockIndexes(editedBounds);

    for (auto index : affectedIndexes)
    {
      if (!m_blocks.contains(index))
      {
        if(value == this->backgroundValue())
        {
          continue;
        }

        createBlock(index);
      }

      auto block       = m_blocks[index];
      auto blockBounds = blockIntersection(block, editedBounds);

      auto bit = itkImageIterator<T>(block, blockBounds);

      while (!bit.IsAtEnd())
      {
        bit.Set(value);

        ++bit;
      }

      if (value == this->backgroundValue() && isEmpty(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::resize(const Bounds &bounds)
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    this->m_bounds = VolumeBounds(bounds, spacing, origin);

    auto affectedIndexes = toBlockIndexes(bounds);
    auto indexes = m_blocks.keys(); // to avoid deleting keys while iterating m_blocks.
    indexes.detach();

    for(auto index: indexes)
    {
      if(!affectedIndexes.contains(index))
      {
        m_blocks[index] = nullptr;
        m_blocks.remove(index);
      }
      else
      {
         auto block = m_blocks[index];
         auto blockRegion = block->GetLargestPossibleRegion();
         auto blockBounds = equivalentBounds<T>(block, blockRegion);
         auto blockIntersection = intersection(bounds, blockBounds);

         // if the block is completely inside the new bounds there is no need to delete anything
         if(blockIntersection == blockBounds) continue;

         // clear voxels outside the new bounds
         auto validRegion = equivalentRegion<T>(block, blockIntersection);
         Q_ASSERT(blockRegion.IsInside(validRegion));
         itk::ImageRegionExclusionIteratorWithIndex<T> iit(block, blockRegion);
         iit.SetExclusionRegion(validRegion);
         iit.GoToBegin();
         while(!iit.IsAtEnd())
         {
           iit.Set(SEG_BG_VALUE);
           ++iit;
         }
      }
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isValid() const
  {
    return this->m_bounds.areValid() && !this->needFetch();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
  {
    QWriteLocker lock(&m_blockMutex);

    if(!m_blocks.empty()) return true; // test

    bool dataFetched = true;

    int i = 0;
    QFileInfo blockFile;

    // Bounds need to be updated before any possible drawing with invalid bounds
    if(bounds.areValid())
    {
      this->m_bounds = bounds;
    }

    if (!storage || path.isEmpty() || id.isEmpty()) return false;

    for (auto filename : {multiBlockPath    (id, i),
                          singleBlockPath   (id),
                          oldSingleBlockPath(id),
                          oldMultiBlockPath (id, i)})
    {
      blockFile = QFileInfo(storage->absoluteFilePath(path + filename));
      if (blockFile.exists()) break;
    }

    auto regions = this->editedRegions();

    while (blockFile.exists())
    {
      typename T::Pointer image = nullptr;

      try
      {
        image = readVolume<T>(blockFile.absoluteFilePath());
      }
      catch(...)
      {
        // NOTE: return 'dataFetched = false' for now when a file/part is bad to force a filter rerun
        // instead of crashing.
        dataFetched = false;
        break;
      }

      auto spacing = this->m_bounds.spacing();
      image->SetSpacing(ItkSpacing<T>(spacing));
      image->Update();

      auto region = image->GetLargestPossibleRegion();
      auto size   = region.GetSize();
      auto origin = image->GetOrigin();

      itkVolumeType::IndexType index;
      for(unsigned int i = 0; i < itkVolumeType::GetImageDimension(); ++i)
      {
        index.SetElement(i, region.GetIndex(i) + vtkMath::Round(origin.GetElement(i)/spacing[i]));
      }

      auto key = lliVector3{index[0]/s_blockSize, index[1]/s_blockSize, index[2]/s_blockSize};

      if (!m_blocks.contains(key)
        && (size[0] == s_blockSize)
        && (size[1] == s_blockSize)
        && (size[2] == s_blockSize)
        && (index[0] % s_blockSize == 0)
        && (index[1] % s_blockSize == 0)
        && (index[2] % s_blockSize == 0))
      {
        m_blocks[key] = image;
      }
      else
      {
        this->draw(image);
      }

      ++i;

      for (auto filename : {multiBlockPath   (id, i),
                            oldMultiBlockPath(id, i)})
      {
        blockFile = QFileInfo(storage->absoluteFilePath(path + filename));
        if (blockFile.exists()) break;
      }

      dataFetched &= true;
    }

    this->setEditedRegions(regions);

    // FIX: 2.1.9 isEmpty() error. Reconstruct the bounds if possible to avoid crashing.
    if(!this->m_bounds.areValid() && (this->m_blocks.keys().size() != 0))
    {
      Bounds tempBounds;
      auto spacing = bounds.spacing();
      for(auto index: this->m_blocks.keys())
      {
        auto region = m_blocks[index]->GetLargestPossibleRegion();
        Bounds bBounds{index[0] * spacing[0] - spacing[0]/2, (index[0] + s_blockSize) * spacing[0]  - spacing[0]/2,
                       index[1] * spacing[1] - spacing[1]/2, (index[1] + s_blockSize) * spacing[1]  - spacing[1]/2,
                       index[2] * spacing[2] - spacing[2]/2, (index[2] + s_blockSize) * spacing[2]  - spacing[2]/2};

        if(!tempBounds.areValid())
        {
          tempBounds = bBounds;
        }
        else
        {
          tempBounds = boundingBox(tempBounds, bBounds, this->m_bounds.spacing());
        }
      }

      this->m_bounds = VolumeBounds{tempBounds, this->m_bounds.spacing(), this->m_bounds.origin()};
    }

    dataFetched &= !m_blocks.isEmpty();

    return dataFetched;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id)
  {
    Snapshot snapshot;

    int i = 0;
    for(auto it = m_blocks.cbegin(); it != m_blocks.cend(); ++it, ++i)
    {
      auto filename = multiBlockPath(id, i);

      if(std::is_same<T, itkVolumeType>::value)
      {
        snapshot << blockSnapshotMemory(it.value(), storage, path, filename);
      }
      else
      {
        snapshot << createVolumeSnapshot<T>(it.value(), storage, path, filename);
      }
    }

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::blockSnapshotMemory(typename T::Pointer volume, TemporalStorageSPtr storage, const QString &path, const QString &id)
  {
    static const QString MHDheader = "ObjectType = Image\nNDims = 3\nBinaryData = True\nBinaryDataByteOrderMSB = False\nCompressedData = False\n"
                                     "TransformMatrix = 1 0 0 0 1 0 0 0 1\nOffset = %1 %2 %3\nCenterOfRotation = 0 0 0\nAnatomicalOrientation = RAI\n"
                                     "ElementSpacing = %4 %5 %6\nITK_InputFilterName = MetaImageIO\nDimSize = %7 %8 %9\nElementType = MET_UCHAR\n"
                                     "ElementDataFile = %10";

    Snapshot snapshot;

    if(volume)
    {
      volume->Update();

      QString mhd = path + id;
      QString raw = mhd;
      raw.replace(".mhd",".raw");

      const auto spacing = volume->GetSpacing();
      const auto origin  = volume->GetOrigin();
      const auto region  = volume->GetLargestPossibleRegion();
      const auto size    = region.GetSize();
      const auto name    = QDir::fromNativeSeparators(storage->absoluteFilePath(raw)).split('/').last();

      const int totalSize = size[0]*size[1]*size[2];

      const double offset[3]{origin.GetElement(0) + (region.GetIndex(0) * spacing[0]),
                             origin.GetElement(1) + (region.GetIndex(1) * spacing[1]),
                             origin.GetElement(2) + (region.GetIndex(2) * spacing[2])};

      auto header = MHDheader.arg(offset[0]).arg(offset[1]).arg(offset[2])
                             .arg(spacing[0]).arg(spacing[1]).arg(spacing[2])
                             .arg(size[0]).arg(size[1]).arg(size[2])
                             .arg(name);

      header = header.replace(',','.');

      QByteArray data(totalSize,0);
      std::memcpy(data.data(), volume->GetBufferPointer(), totalSize);

      snapshot << SnapshotData(mhd, header.toLatin1());
      snapshot << SnapshotData(raw, data);
    }

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    Snapshot regionsSnapshot;

    BoundsList regions = this->editedRegions();
    // TODO: Simplify edited regions volumes

    int regionId = 0;
    for(auto region : regions)
    {
      auto editedBounds = intersection(region, this->m_bounds);
      if (editedBounds.areValid())
      {
        auto snapshotId = editedRegionSnapshotId(id, regionId);

        if(std::is_same<T, itkVolumeType>::value)
        {
          regionsSnapshot << blockSnapshotMemory(itkImage(editedBounds), storage, path, snapshotId);
        }
        else
        {
          regionsSnapshot << createVolumeSnapshot<T>(itkImage(editedBounds), storage, path, snapshotId);
        }

        ++regionId;
      }
    }

    return regionsSnapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    auto restoredEditedRegions = this->editedRegions();

    auto spacing = ItkSpacing<T>(this->m_bounds.spacing());

    for (int regionId = 0; regionId < restoredEditedRegions.size(); ++regionId)
    {
      QFileInfo filename(storage->absoluteFilePath(path + editedRegionSnapshotId(id, regionId)));

      if (filename.exists())
      {
        auto editedRegion = readVolume<itkVolumeType>(filename.absoluteFilePath());

        changeSpacing<T>(editedRegion, spacing);

        expandAndDraw<T>(this, editedRegion);
      }
      else
      {
        qWarning() << "Unable to locate edited region file:" << filename.absoluteFilePath();
      }
    }

    this->setEditedRegions(restoredEditedRegions);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isEmpty() const
  {
    if(isValid() && !m_blocks.isEmpty())
    {
      for(auto index: this->m_blocks.keys())
      {
        if(!isEmpty(index))
        {
          return false;
        }
      }
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  std::shared_ptr<SparseVolume<T>> sparseCopy(typename T::Pointer image)
  {
    auto bounds  = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
    auto spacing = ToNmVector3<T>(image->GetSpacing());
    auto origin  = ToNmVector3<T>(image->GetOrigin());

    auto sparse = std::make_shared<SparseVolume<T>>(bounds, spacing, origin);

    sparse->draw(image, bounds);

    return sparse;
  }

  using SparseVolumePtr  = SparseVolume<itkVolumeType> *;
  using SparseVolumeSPtr = std::shared_ptr<SparseVolume<itkVolumeType>>;

  //-----------------------------------------------------------------------------
  template<typename T>
  typename SparseVolume<T>::BlockIndexes SparseVolume<T>::toBlockIndexes(const itk::ImageRegion<3> &region) const
  {
    BlockIndexes result;
    lliVector3 minimum, maximum;

    auto index  = region.GetIndex();
    auto size   = region.GetSize();

    for(int i = 0; i < 3; ++i)
    {
      minimum[i] = vtkMath::Floor(index[i]/static_cast<double>(this->s_blockSize));
      maximum[i] = vtkMath::Ceil((index[i]+static_cast<int>(size[i]))/static_cast<double>(this->s_blockSize));
    }

    for(int i = minimum[0]; i < maximum[0]; ++i)
    {
      for(int j = minimum[1]; j < maximum[1]; ++j)
      {
        for(int k = minimum[2]; k < maximum[2]; ++k)
        {
          result << lliVector3{i,j,k};
        }
      }
    }

    return result;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  typename SparseVolume<T>::BlockIndexes SparseVolume<T>::toBlockIndexes(const Bounds &bounds) const
  {
    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    return bounds.areValid() ? toBlockIndexes(equivalentRegion<T>(origin, spacing, bounds)) : BlockIndexes();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds SparseVolume<T>::editRegion(const VolumeBounds &bounds)
  {
    Bounds editionBounds;

    if (intersect(this->m_bounds, bounds))
    {
      editionBounds = intersection(this->m_bounds, bounds);

      this->addEditedRegion(editionBounds);
      this->updateModificationTime();
    }

    return editionBounds;
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::createBlock(const lliVector3 &index)
  {
    itk::ImageRegion<3> region;
    region.SetIndex(0, index[0] * this->s_blockSize);
    region.SetIndex(1, index[1] * this->s_blockSize);
    region.SetIndex(2, index[2] * this->s_blockSize);
    region.SetSize(0, this->s_blockSize);
    region.SetSize(1, this->s_blockSize);
    region.SetSize(2, this->s_blockSize);

    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();

    auto bounds = equivalentBounds<T>(origin, spacing, region);
    bounds.setLowerInclusion(true);
    bounds.setUpperInclusion(false);

    m_blocks[index] = create_itkImage<T>(bounds, this->backgroundValue(), spacing, origin);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isEmpty(const lliVector3 &index) const
  {
    auto block  = m_blocks[index];
    auto region = block->GetLargestPossibleRegion();
    auto it     = itk::ImageRegionConstIterator<T>(block, region);

    it.GoToBegin();
    while(!it.IsAtEnd())
    {
      if(it.Value() != SEG_BG_VALUE)
      {
        return false;
      }
      ++it;
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds SparseVolume<T>::blockIntersection(typename T::Pointer block, const Bounds &bounds) const
  {
    auto blockBounds = equivalentBounds<T>(block);
    return intersection(blockBounds, bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::RegionType SparseVolume<T>::itkRegion() const
  {
    return equivalentRegion<T>(this->m_bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::SpacingType SparseVolume<T>::itkSpacing() const
  {
    if(!this->m_blocks.isEmpty())
    {
      return m_blocks.first()->GetSpacing();
    }

    typename T::SpacingType spacing;
    spacing.Fill(0);

    return spacing;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::PointType SparseVolume<T>::itkOriginalOrigin() const
  {
    typename T::PointType origin;
    origin.Fill(0);

    return origin;
  }

}

#endif // ESPINA_SPARSE_VOLUME_H
