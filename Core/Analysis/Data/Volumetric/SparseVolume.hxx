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
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/Vector3.hxx>
#include <Core/Utils/SpatialUtils.hxx>

// ITK
#include <itkImageRegionIterator.h>
#include <itkExtractImageFilter.h>
#include <itkMetaImageIO.h>
#include <itkImageFileReader.h>
#include <itkConstantPadImageFilter.h>

// VTK
#include <vtkImplicitFunction.h>

// Qt
#include <QMap>
#include <QReadWriteLock>

namespace ESPINA
{
  struct Invalid_Image_Bounds_Exception{};
  struct Invalid_Image_Output_Value_Exception{};

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
    using BlockMask     = BinaryMask<unsigned char>;
    using BlockMaskPtr  = BlockMask*;
    using BlockMaskSPtr = std::shared_ptr<BlockMask>;

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

    virtual Bounds bounds() const override;

    virtual void setOrigin(const NmVector3& origin) override;

    virtual NmVector3 origin() const override;

    virtual void setSpacing(const NmVector3& spacing) override;

    virtual NmVector3 spacing() const override
    { return m_spacing; }

    virtual const typename T::Pointer itkImage() const override;

    virtual const typename T::Pointer itkImage(const Bounds& bounds) const override;

    virtual void draw(const vtkImplicitFunction*  brush,
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

    virtual bool isValid() const override;

    virtual bool isEmpty() const override;

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const              override;

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override;

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)            override;

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override;

  private:
    QString editedRegionSnapshotId(const QString &outputId, const int regionId) const
    { return QString("%1_%2_EditedRegion_%3.mhd").arg(outputId).arg(this->type()).arg(regionId);}

    /** \brief Returns the list of block indexes that comprises (total or partially)
     * the region passed as the argument.
     * \param[in] bounds bounds.
     *
     */
    QList<liVector3> toBlockIndexes(const itk::ImageRegion<3> &region) const;

    /** \brief Returns the list of block indexes that comprises (total or partially)
     * the bounds passed as the argument.
     * \param[in] bounds bounds.
     *
     */
    QList<liVector3> toBlockIndexes(const Bounds &bounds) const;

    /** \brief Creates a block for the given index
     * \param[in] index block index.
     *
     */
    void createBlock(const liVector3 &index);

    /** \brief Returns true if the block associated with the given index is empty.
     * \param[in] index block index.
     *
     */
    bool isEmpty(const liVector3 &index) const;

  private:
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

  protected:
    NmVector3    m_origin;
    NmVector3    m_spacing;
    VolumeBounds m_bounds;

    const unsigned int s_blockSize = 25;
    QMap<liVector3, typename T::Pointer> m_blocks;

    mutable QReadWriteLock m_blockMutex;
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  , m_origin   {origin}
  , m_spacing  {spacing}
  {
    m_bounds = VolumeBounds(bounds, m_spacing, m_origin);

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
  Bounds SparseVolume<T>::bounds() const
  {
    return m_bounds.bounds();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setOrigin(const NmVector3& origin)
  {
    //NOTE: 2015-04-20 Review when tiling support added
    //NmVector3 shift = m_origin - origin;
    m_origin = origin;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  NmVector3 SparseVolume<T>::origin() const
  {
    return m_origin;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setSpacing(const NmVector3& spacing)
  {
    if (m_spacing != spacing)
    {
      auto itkSpacing = ItkSpacing<T>(spacing);

      NmVector3 spacingRatio;
      for (int i = 0; i < 3; ++i)
      {
        spacingRatio[i] = spacing[i]/m_spacing[i];
      }

      QMapIterator<liVector3, typename T::Pointer> it(m_blocks);
      while(it.hasNext())
      {
        it.next();
        auto block  = it.value();
        auto origin = block->GetOrigin();

        for (int i = 0; i < 3; ++i)
        {
          origin[i] *= spacingRatio[i];
        }

        //auto preBlockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
        block->SetOrigin(origin);
        block->SetSpacing(itkSpacing);
        block->Update();
        //auto postBlockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
        //qDebug() << "Update block bounds" << preBlockBounds << "to"<< postBlockBounds;
      }

      auto region = equivalentRegion<T>(m_origin, m_spacing, m_bounds);

      m_spacing = spacing;

      m_bounds = volumeBounds<T>(m_origin, spacing, region);
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage() const
  {
    return itkImage(m_bounds.bounds());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage(const Bounds& bounds) const
  {
    if (!bounds.areValid())
    {
      throw Invalid_Image_Bounds_Exception();
    }

    VolumeBounds expectedBounds(bounds, m_spacing, m_origin);

    if (!contains(m_bounds, expectedBounds))
    {
      qDebug() << m_bounds;
      qDebug() << expectedBounds;
      throw Invalid_Image_Bounds_Exception();
    }

    m_blockMutex.lockForRead();

    auto image = create_itkImage<T>(bounds, this->backgroundValue(), m_spacing, m_origin);
    auto affectedIndexes = toBlockIndexes(bounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index)) continue;

      auto block = m_blocks[index];

      auto blockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto intersectionBounds = intersection(blockBounds, expectedBounds);

      auto iit = itkImageIterator<T>(image, intersectionBounds);
      auto bit = itkImageConstIterator<T>(block, intersectionBounds);

      while(!iit.IsAtEnd())
      {
        iit.Set(bit.Value());

        ++iit;
        ++bit;
      }
    }

    m_blockMutex.unlock();

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const vtkImplicitFunction  *brush,
                             const Bounds               &bounds,
                             const typename T::ValueType value)
  {
    VolumeBounds requestedBounds(bounds, m_spacing, m_origin);

    if (!intersect(m_bounds, requestedBounds))
    {
      return;
    }

    auto intersectionBounds = intersection(m_bounds, requestedBounds).bounds();
    auto affectedIndexes    = toBlockIndexes(intersectionBounds);

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

      auto block = m_blocks[index];
      auto blockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto blockIntersection = intersection(blockBounds, intersectionBounds);

      auto bit = itkImageIteratorWithIndex<T>(block, blockIntersection);
      while(!bit.IsAtEnd())
      {
        auto index = bit.GetIndex();
        NmVector3 point{index[0]*m_spacing[0] + m_spacing[0]/2,
                        index[1]*m_spacing[1] + m_spacing[1]/2,
                        index[2]*m_spacing[2] + m_spacing[2]/2};

        if (const_cast<vtkImplicitFunction *>(brush)->FunctionValue(point[0], point[1], point[2]) <= 0)
        {
          bit.Set(value);
        }

        ++bit;
      }

      if(value == this->backgroundValue() && isEmpty(index))
      {
        m_blocks.remove(index);
      }
    }

    this->addEditedRegion(intersectionBounds);
    this->updateModificationTime();
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
    VolumeBounds requestedBounds(mask->bounds(), m_spacing, m_origin);

    if (!intersect(m_bounds, requestedBounds))
    {
      return;
    }

    auto intersectionBounds = intersection(m_bounds, requestedBounds).bounds();
    auto affectedIndexes = toBlockIndexes(intersectionBounds);

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

      auto block = m_blocks[index];
      auto blockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto blockIntersection = intersection(blockBounds, intersectionBounds);

      BinaryMask<unsigned char>::region_iterator mit(mask.get(), blockIntersection);
      mit.goToBegin();
      auto bit = itkImageIteratorWithIndex<T>(block, blockIntersection);

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
        m_blocks.remove(index);
      }
    }

    this->addEditedRegion(intersectionBounds);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer image)
  {
    Bounds bounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());

    VolumeBounds requestedBounds(bounds, m_spacing, m_origin);
    if (!intersect(m_bounds, requestedBounds))
    {
      return;
    }

    auto intersectionBounds = intersection(m_bounds, requestedBounds).bounds();
    auto affectedIndexes = toBlockIndexes(intersectionBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        createBlock(index);
      }

      auto block             = m_blocks[index];
      auto blockBounds       = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto blockIntersection = intersection(blockBounds, intersectionBounds);

      auto iit = itkImageConstIterator<T>(image, blockIntersection);
      auto bit = itkImageIterator<T>(block, blockIntersection);

      while(!iit.IsAtEnd())
      {
        bit.Set(iit.Value());

        ++iit;
        ++bit;
      }

      if(isEmpty(index))
      {
        m_blocks.remove(index);
      }
    }

    this->addEditedRegion(intersectionBounds);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer image,
                             const Bounds&             bounds)
  {
    VolumeBounds requestedBounds(bounds, m_spacing, m_origin);

    if (!intersect(m_bounds, requestedBounds))
    {
      return;
    }

    auto intersectionBounds = intersection(m_bounds, requestedBounds).bounds();
    auto affectedIndexes = toBlockIndexes(intersectionBounds);

    for(auto index: affectedIndexes)
    {
      if(!m_blocks.contains(index))
      {
        createBlock(index);
      }

      auto block = m_blocks[index];
      auto blockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto blockIntersection = intersection(blockBounds, intersectionBounds);

      auto iit = itkImageConstIterator<T>(image, blockIntersection);
      auto bit = itkImageIterator<T>(block, blockIntersection);

      while(!iit.IsAtEnd())
      {
        bit.Set(iit.Value());

        ++iit;
        ++bit;
      }

      if(isEmpty(index))
      {
        m_blocks.remove(index);
      }
    }

    this->addEditedRegion(intersectionBounds);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const typename T::IndexType &index,
                             const typename T::ValueType  value)
  {
    Bounds bounds { index[0] * m_spacing[0], index[0] * m_spacing[0],
                    index[1] * m_spacing[1], index[1] * m_spacing[1],
                    index[2] * m_spacing[2], index[2]	* m_spacing[2] };

    draw(bounds, value);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const Bounds               &bounds,
                             const typename T::ValueType value)
  {
    VolumeBounds requestedBounds(bounds, m_spacing, m_origin);

    if (!intersect(m_bounds, requestedBounds))
    {
      return;
    }

    auto intersectionBounds = intersection(m_bounds, requestedBounds).bounds();
    auto affectedIndexes = toBlockIndexes(intersectionBounds);

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

      auto block = m_blocks[index];
      auto blockBounds = equivalentBounds<T>(block, block->GetLargestPossibleRegion());
      auto blockIntersection = intersection(blockBounds, intersectionBounds);

      auto bit = itkImageIterator<T>(block, blockIntersection);

      while (!bit.IsAtEnd())
      {
        bit.Set(value);

        ++bit;
      }

      if (value == this->backgroundValue() && isEmpty(index))
      {
        m_blocks.remove(index);
      }
    }

    this->addEditedRegion(intersectionBounds);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::resize(const Bounds &bounds)
  {
    m_bounds = VolumeBounds(bounds, m_spacing, m_origin);

    auto affectedIndexes = toBlockIndexes(bounds);

    for(auto index: m_blocks.keys())
    {
      if(!affectedIndexes.contains(index))
      {
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
    return m_bounds.areValid() && !this->needFetch();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
  {
    QWriteLocker lock(&m_blockMutex);

    using VolumeReader = itk::ImageFileReader<itkVolumeType>;

    bool dataFetched = false;

    int i = 0;
    QFileInfo blockFile;

    // Bounds need to be updated before any possible drawing with invalid bounds
    if(bounds.areValid())
    {
      m_bounds  = bounds;
      m_spacing = bounds.spacing();
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

    while (blockFile.exists())
    {
      VolumeReader::Pointer reader = VolumeReader::New();
      reader->SetFileName(blockFile.absoluteFilePath().toUtf8().data());

      try
      {
        reader->Update();
      }
      catch(...)
      {
        // NOTE: return 'dataFetched = false' for now when a file/part is bad to force a filter rerun
        // instead of crashing.
        break;
      }

      auto image = reader->GetOutput();
      image->SetSpacing(ItkSpacing<T>(m_spacing));
      image->Update();

      auto bounds      = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
      auto blockRegion = equivalentRegion<T>(m_origin, m_spacing, bounds);

      auto size  = blockRegion.GetSize();
      auto index = blockRegion.GetIndex();
      auto key   = liVector3{index[0]/s_blockSize, index[1]/s_blockSize, index[2]/s_blockSize};

      if (!m_blocks.contains(key) &&
          (size[0] == s_blockSize) && (size[1] == s_blockSize) && (size[2] == s_blockSize) &&
          (index[0] % s_blockSize == 0) && (index[1] % s_blockSize == 0) && (index[2] % s_blockSize == 0))
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

      dataFetched = true;
    }

    return dataFetched;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
  {
    Snapshot snapshot;

    QMapIterator<liVector3, typename T::Pointer> it(m_blocks);
    int i = 0;
    while(it.hasNext())
    {
      it.next();

      auto filename = multiBlockPath(id, i++);

      snapshot << createSnapshot<T>(it.value(), storage, path, filename);
    }

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
  {
    Snapshot regionsSnapshot;

    BoundsList regions = this->editedRegions();
    // TODO: Simplify edited regions volumes

    int regionId = 0;
    for(auto region : regions)
    {
      auto editedBounds = intersection(region, bounds());
      if (editedBounds.areValid())
      {
        auto snapshotId    = editedRegionSnapshotId(id, regionId);
        regionsSnapshot << createSnapshot<T>(itkImage(editedBounds), storage, path, snapshotId);
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

    for (int regionId = 0; regionId < restoredEditedRegions.size(); ++regionId)
    {
      QFileInfo filename(storage->absoluteFilePath(path + editedRegionSnapshotId(id, regionId)));

      if (filename.exists())
      {
        auto editedRegion = readVolume<itkVolumeType>(filename.absoluteFilePath());

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
    return (!isValid() || m_blocks.isEmpty());
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
  QList<liVector3> SparseVolume<T>::toBlockIndexes(const itk::ImageRegion<3> &region) const
  {
    QList<liVector3> result;
    liVector3 minimum, maximum;

    auto index  = region.GetIndex();
    auto size   = region.GetSize();
//     qDebug() << "Partitioning Region";
//     region.Print(std::cout);

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
          result << liVector3{i,j,k};
        }
      }
    }
    //qDebug() << result;

    return result;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  QList<liVector3> SparseVolume<T>::toBlockIndexes(const Bounds &bounds) const
  {
    return toBlockIndexes(equivalentRegion<T>(m_origin, m_spacing, bounds));
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::createBlock(const liVector3 &index)
  {
    itk::ImageRegion<3> region;
    region.SetIndex(0, index[0] * this->s_blockSize);
    region.SetIndex(1, index[1] * this->s_blockSize);
    region.SetIndex(2, index[2] * this->s_blockSize);
    region.SetSize(0, this->s_blockSize);
    region.SetSize(1, this->s_blockSize);
    region.SetSize(2, this->s_blockSize);

    auto bounds = equivalentBounds<T>(m_origin, m_spacing, region);
    bounds.setLowerInclusion(true);
    bounds.setUpperInclusion(false);

    m_blocks[index] = create_itkImage<T>(bounds, this->backgroundValue(), m_spacing, m_origin);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isEmpty(const liVector3 &index) const
  {
    auto block  = m_blocks[index];
    auto region = block->GetLargestPossibleRegion();
    auto it     = itk::ImageRegionIterator<T>(block, region);
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
}

#endif // ESPINA_SPARSE_VOLUME_H
