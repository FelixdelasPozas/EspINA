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

// ITK
#include <itkImageRegionIterator.h>
#include <itkExtractImageFilter.h>
#include <itkMetaImageIO.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

// VTK
#include <vtkImplicitFunction.h>

namespace ESPINA
{
  struct Invalid_Image_Bounds_Exception{};

  /*! \brief Volume representation intended to save memory and speed up
   *  edition operations
   *
   *  Those voxel which don't belong to any block are assigned the value
   *  defined as background value.
   *
   *  Add operation will replace every voxel with a value different to
   *  the background value.
   */
  template<typename T>
  class EspinaCore_EXPORT SparseVolume
  : public VolumetricData<T>
  {
    using BlockMask     = BinaryMask<unsigned char>;
    using BlockMaskPtr  = BlockMask*;
    using BlockMaskSPtr = std::shared_ptr<BlockMask>;

    const int UNSET = 0;
    const int SET   = 1;

  public:
    /** \brief SparseVolume class constructor
     * \param[in] bounds, bounds of the empty volume.
     * \param[in] spacing, spacing of the volume.
     * \param[in] origin, origin of the volume.
     *
     */
    explicit SparseVolume(const Bounds& bounds = Bounds(), const NmVector3& spacing = {1, 1, 1}, const NmVector3& origin = NmVector3());

    /** \brief SparseVolume class destructor.
     *
     */
    virtual ~SparseVolume()
    {}

    /** \brief Implements Data::memoryUsage() const.
     *
     */
    virtual size_t memoryUsage() const;

    /** \brief Implements Data::bounds() const.
     *
     */
    virtual Bounds bounds() const
    { return m_bounds.bounds(); }

    /** \brief Sets the origin of the volume.
     *
     */
    virtual void setOrigin(const NmVector3& origin);

    /** \brief Returns the origin of the volume.
     *
     */
    virtual NmVector3 origin() const
    { return m_origin; }

    /** \brief Implements Data::setSpacing().
     *
     */
    virtual void setSpacing(const NmVector3& spacing);

    /** \brief Implements Data::spacing().
     *
     */
    virtual NmVector3 spacing() const
    { return m_spacing; }

    /** \brief Returns the equivalent itk image of the volume.
     *
     */
    virtual const typename T::Pointer itkImage() const;

    /** \brief Returns the equivalent itk image of a region of the volume.
     * \param[in] bounds, equivalent bounds of the returning image.
     *
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    /** \brief Method to modify the volume using a implicit function.
     * \param[in] brush, vtkImplicitFuncion object raw pointer.
     * \param[in] bounds, bounds to be modified (must be contained in the image bounds).
     * \param[in] value, value of the voxel.
     *
     *  Change every voxel value which satisfies the implicit function to
     *  the value given as parameter.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /** \brief Method to modify the volume using a mask and a value.
     * \param[in] mask, BinatyMask smart pointer.
     * \param[in] value, value of the voxels of the binary mask.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /** \brief Method to modify the volume using an itk image.
     * \param[in] volume, itk image smart pointer to draw.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const typename T::Pointer volume);

    /** \brief Method to modify the volume using a region of an itk image.
     * \param[in] volume, itk image smart pointer to draw.
     * \param[in] bounds, bounds affected by the draw operation.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds);

    /** \brief Method to modify a voxel of the volume using an itk index.
     * \param[in] index, index of the voxel.
     * \param[in] value, value of the voxel.
     *
     *  Draw methods are constrained to volume bounds.
     */
    virtual void draw(const typename T::IndexType index,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /** \brief Resize the volume bounds. The given bounds must containt the original.
     * \param[in] bounds, bounds object.
     *
     */
    virtual void resize(const Bounds &bounds);

    /** \brief Method to undo the last change made to the volume.
     *
     */
    virtual void undo();

    /** \brief Implements Data::isValid() const.
     *
     */
    virtual bool isValid() const;

    /** \brief Implements Data::isEmpty() const.
     *
     */
    virtual bool isEmpty() const;

    /** \brief Implements Data::fetchData().
     *
     */
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix);

    /** \brief Implements Data::snapshot() const.
     *
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const;

    /** \brief Implements Data::editedRegionsSnapshot() const.
     *
     */
    virtual Snapshot editedRegionsSnapshot() const
    { return Snapshot(); }

  protected:
    /** \brief Replace sparse volume voxels within data region with data voxels
     *
     *  Sparse Volume will take ownership of the block.
     *  The isLocked parameter signals if the block has been read from disk or is a
     *  block originated from the session (a modification).
     */
    void setBlock(typename T::Pointer image, bool isLocked = false);

    /** \brief Set non background data voxels of sparse volume to data voxel values
     *
     *  For every voxel in data, set the value of its equivalent sparse volume voxel to
     *  the value of the data voxel
     *  Sparse Volume will take ownership of the block
     */
    void addBlock(BlockMaskSPtr mask);

    /** \brief Returns a list of contiguous VolumeBounds where the image is not empty.
     *
     */
    VolumeBoundsList compactedBlocks() const;

  private:
    enum class BlockType
    { Set = 0, Add = 1 };

    class Block;
    using BlockSPtr = std::shared_ptr<Block>;
    using BlockList = std::vector<BlockSPtr>;

    class Block
    {
      public:
        struct Invalid_Iteration_Bounds_Exception{};

      public:
     		/** \brief Block class constructor.
     		 * \param[in] locked, true if this image will not be affected by undo() operation.
     		 *
     		 */
        explicit Block(bool locked)
        : m_locked{locked}
        {}

        /** \brief Block class destructor.
         *
         */
        virtual ~Block()
        {}

        /** \brief Returns the VolumeBounds of the block.
         *
         */
        virtual VolumeBounds bounds() const = 0;

        /** \brief Sets the spacing of the block.
         *
         */
        virtual void setSpacing(const NmVector3& spacing) = 0;

        /** \brief Returns the memory consumption for the block in bytes.
         *
         */
        virtual size_t memoryUsage() const = 0;

        /** \brief Updates the image passed as parameter with the values of the block.
         * \param[in] iit, external image region iterator.
         * \param[in] mit, external mask region iterator.
         * \param[in] bounds, bounds of the modification.
         * \param[in] remainingVoxels, remaining voxels of the mask to be updated.
         *
         */
        virtual void updateImage(itk::ImageRegionIterator<T>                &iit,
                                 BinaryMask<unsigned char>::region_iterator &mit,
                                 const Bounds                              &bounds,
                                 long unsigned int                         &remainingVoxels) const = 0;

        /** \brief Returns true if the block will be unaffected by a undo() operation.
         *
         */
        bool isLocked() const
        { return m_locked; }

      protected:
        friend class SparseVolume;

        bool          m_locked;
    };

    class MaskBlock
    : public Block
    {
    public:
    	/** \brief MaskBlock class constructor.
    	 * \param[in] mask, BinaryMask smart pointer.
    	 * \param[in] locked, true if this image will not be affected by undo() operation.
    	 *
    	 */
      MaskBlock(BlockMaskSPtr mask, bool locked)
      : Block(locked)
      , m_mask{mask}
      {}

      /** \brief Implements Block::bounds() const.
       *
       */
      virtual VolumeBounds bounds() const
      { return m_mask->bounds(); }

      /** \brief Implements Block::setSpacing().
       *
       */
      virtual void setSpacing(const NmVector3& spacing)
      { m_mask->setSpacing(spacing); }

      /** \brief Implements Block::memoryUsage() const.
       *
       */
      virtual size_t memoryUsage() const
      { return m_mask->memoryUsage(); }

      /** \brief Implements Block::updateImage() const.
       *
       */
      virtual void updateImage(itk::ImageRegionIterator<T>                &iit,
                               BinaryMask<unsigned char>::region_iterator &mit,
                               const Bounds                              &bounds,
                               long unsigned int                         &remainingVoxels) const
      {
        BinaryMask<unsigned char>::region_iterator bit(this->m_mask.get(), bounds);
        bit.goToBegin();

        while(!mit.isAtEnd())
        {
          if (!mit.isSet() && bit.isSet())
          {
            iit.Set(this->m_mask->foregroundValue());
            mit.Set();
            --remainingVoxels;
          }
          ++mit;
          ++bit;
          ++iit;
        }
      }

      BlockMaskSPtr m_mask;
    };

    class ImageBlock
    : public Block
    {
    public:
   		/** \brief ImageBlock class constructor.
   		 * \param[in] image, itk image smart pointer.
   		 * \param[in] locked, true if this image will not be affected by undo() operation.
   		 *
   		 */
      ImageBlock(typename T::Pointer image, bool locked)
      : Block(locked)
      , m_image{image}
      {}

      /** \brief Implements Block::bounds() const.
       *
       */
      virtual VolumeBounds bounds() const
      {
        return volumeBounds<T>(m_image, m_image->GetLargestPossibleRegion());
      }

      /** \brief Implements Block::setSpacing().
       *
       */
      virtual void setSpacing(const NmVector3& spacing)
      { m_image->SetSpacing(ItkSpacing<T>(spacing)); }

      /** \brief Implements Block::memoryUsage().
       *
       */
      virtual size_t memoryUsage() const
      { return m_image->GetBufferedRegion().GetNumberOfPixels()*sizeof(typename T::ValueType); }

      /** \brief Implements Block::updateImage() const.
       *
       */
      virtual void updateImage(itk::ImageRegionIterator<T>                &iit,
                               BinaryMask<unsigned char>::region_iterator &mit,
                               const Bounds                              &bounds,
                               long unsigned int                         &remainingVoxels) const
      {
        auto blockRegion = equivalentRegion<T>(m_image, bounds);
        itk::ImageRegionIterator<T> bit(m_image, blockRegion);
        bit.GoToBegin();
        while(!mit.isAtEnd())
        {
          if (!mit.isSet())
          {
            iit.Set(bit.Value());
            mit.Set();
            --remainingVoxels;
          }
          ++mit;
          ++bit;
          ++iit;
        }
      }

      typename T::Pointer m_image;
    };

  private:
    /** \brief Partitions the bounds of the image in a list of VolumeBounds
     * whose size is, at most, intevalSize*intervalSize*intervalSize.
     *
     */
    VolumeBoundsList boundsPartition(int intervalSize) const;

    /** \brief Updates the bounds of the image with the bounds passed as parameter.
     * \param[in] bounds, VolumeBounds object.
     *
     */
    void updateBlocksBoundingBox(const VolumeBounds& bounds);

    /** \brief Helper method to assist fetching data from disk.
     *
     */
    QString singleBlockPath() const
    { return this->type() + QString("_%1.mhd").arg(this->m_output?this->m_output->id():0); }

    /** \brief Helper method to assist fetching data from disk.
     *
     */
    QString multiBlockPath(int part) const
    { return this->type() + QString("_%1_%2.mhd").arg(this->m_output?this->m_output->id():0).arg(part); }

  protected:
    mutable BlockList m_blocks;
    NmVector3 m_origin;
    NmVector3 m_spacing;
    VolumeBounds m_bounds;
    VolumeBounds m_blocks_bounding_box;
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  , m_origin {origin}
  , m_spacing{spacing}
  {
    if (bounds.areValid())
    {
      m_bounds = VolumeBounds(bounds, m_spacing, m_origin);
    }

    this->setBackgroundValue(0);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  size_t SparseVolume<T>::memoryUsage() const
  {
    size_t size = 0;

    for (auto block : m_blocks)
    {
      size += block->memoryUsage();
    }

    return size;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setOrigin(const NmVector3& origin)
  {
    //TODO
    //NmVector3 shift = m_origin - origin;
    m_origin = origin;
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setSpacing(const NmVector3& spacing)
  {
    if (m_spacing != spacing)
    {
      for(auto block : m_blocks)
      {
        block->setSpacing(spacing);
      }

      auto region = equivalentRegion<T>(m_origin, m_spacing, m_bounds.bounds());

      m_spacing = spacing;

      // TODO: use m_bounds.setSpacing()
      m_bounds = volumeBounds<T>(m_origin, m_spacing, region);
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setBlock(typename T::Pointer image, bool isLocked)
  {
    BlockSPtr block(new ImageBlock(image, isLocked));
    m_blocks.push_back(block);

    // DEBUG
//    Bounds bounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
//    auto i2 = T::New();
//    i2->SetSpacing(image->GetSpacing());
//    auto region = equivalentRegion<T>(i2, bounds);
//
//    image->GetLargestPossibleRegion().Print(std::cout);
//    region.Print(std::cout);

    updateBlocksBoundingBox(block->bounds());
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::addBlock(BlockMaskSPtr mask)
  {
    BlockSPtr block(new MaskBlock(mask, false));
    m_blocks.push_back(block);

    updateBlocksBoundingBox(mask->bounds());
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
      throw Invalid_Image_Bounds_Exception();

    VolumeBounds expectedBounds(bounds, m_spacing, m_origin);

    if (!contains(m_bounds, expectedBounds))
    {
      qDebug() << m_bounds;
      qDebug() << expectedBounds;
      throw Invalid_Image_Bounds_Exception();
    }

    auto image     = create_itkImage<T>(bounds, this->backgroundValue(), m_spacing, m_origin);
    auto mask      = new BinaryMask<unsigned char>(bounds, m_spacing, m_origin);
    auto numVoxels = image->GetLargestPossibleRegion().GetNumberOfPixels();

    Q_ASSERT(numVoxels == mask->numberOfVoxels());

    for (int i = m_blocks.size() - 1; i >= 0; --i)
    {
      auto block = m_blocks[i];

      VolumeBounds blockBounds(block->bounds());

      if (!intersect(expectedBounds, blockBounds))
        continue;

      if (numVoxels == 0)
        break;

      Bounds intersectionBounds = intersection(expectedBounds, blockBounds).bounds();

      BinaryMask<unsigned char>::region_iterator mit(mask, intersectionBounds);
      itk::ImageRegionIterator<T> iit(image, equivalentRegion<T>(image, intersectionBounds));
      mit.goToBegin();
      iit.GoToBegin();

      block->updateImage(iit, mit, intersectionBounds, numVoxels);
    }

    delete mask;

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
      return;

    Bounds intersectionBounds = intersection(m_bounds, requestedBounds).bounds();

    BlockMaskSPtr mask{new BinaryMask<unsigned char>(intersectionBounds, m_spacing, m_origin)};
    mask->setForegroundValue(value);

    BinaryMask<unsigned char>::region_iterator it(mask.get(), intersectionBounds);

    it.goToBegin();
    while (!it.isAtEnd())
    {
      NmVector3 point = it.getCenter();

      if (const_cast<vtkImplicitFunction *>(brush)->FunctionValue(point[0], point[1], point[2]) <= 0)
        it.Set();
      ++it;
    }

    addBlock(mask);
    this->updateModificationTime();
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
    addBlock(mask);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer volume)
  {
    Bounds volumeBounds = equivalentBounds<T>(volume, volume->GetLargestPossibleRegion());

    draw(volume, volumeBounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer volume,
                             const Bounds&             bounds)
  {
    VolumeBounds requestedBounds(bounds, m_spacing, m_origin);

    if (!intersect(m_bounds, requestedBounds))
      return;

    VolumeBounds largestBounds = volumeBounds<T>(volume, volume->GetLargestPossibleRegion());

    VolumeBounds inputBounds   = volumeBounds<T>(volume, bounds);

    VolumeBounds drawBounds    = intersection(m_bounds, inputBounds);

    typename T::Pointer block;

    if (drawBounds != largestBounds)
    {
      using ExtractorType = itk::ExtractImageFilter<T,T>;

      auto extractor = ExtractorType::New();
      auto region    = equivalentRegion<T>(volume, drawBounds.bounds());

      extractor->SetInput(volume);
      extractor->SetExtractionRegion(region);
      extractor->SetInPlace(false);
      extractor->SetNumberOfThreads(1);
      extractor->ReleaseDataBeforeUpdateFlagOn();
      extractor->Update();

      block = extractor->GetOutput();
    }
    else
      block = volume;

    block->DisconnectPipeline();

    setBlock(block);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const typename T::IndexType index,
                             const typename T::ValueType value)
  {
		Bounds bounds { index[0] * m_spacing[0], index[0] * m_spacing[0],
    	              index[1] * m_spacing[1], index[1] * m_spacing[1],
    	              index[2] * m_spacing[2], index[2]	* m_spacing[2] };

		if (!intersect(m_bounds.bounds(), bounds))
			return;

		BlockMaskSPtr mask { new BinaryMask<unsigned char>(bounds, m_spacing) };
		mask->setForegroundValue(value);
		BinaryMask<unsigned char>::region_iterator it(mask.get(), mask->bounds().bounds());
		it.goToBegin();
		it.Set();

		addBlock(mask);
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::resize(const Bounds &bounds)
  {
    m_bounds = VolumeBounds(bounds, m_spacing, m_origin);
    if (m_blocks_bounding_box.areValid())
    {
      m_blocks_bounding_box = intersection(m_bounds, m_blocks_bounding_box);
    }

    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isValid() const
  {
    return m_bounds.areValid();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::fetchData(TemporalStorageSPtr storage, const QString& prefix)
  {
    using VolumeReader = itk::ImageFileReader<itkVolumeType>;

    bool dataFetched = false;
    bool error       = false;

    int i = 0;
    QFileInfo blockFile(storage->absoluteFilePath(prefix + multiBlockPath(i)));
    if (!blockFile.exists())
    {
      blockFile = QFileInfo(storage->absoluteFilePath(prefix + singleBlockPath()));
    }

    if (this->m_output)
    {
      m_spacing = this->m_output->spacing();
    }

    auto itkSpacing = ItkSpacing<T>(m_spacing);

    while (blockFile.exists())
    {
      VolumeReader::Pointer reader = VolumeReader::New();
      reader->SetFileName(blockFile.absoluteFilePath().toUtf8().data());
      reader->Update();

      auto image = reader->GetOutput();

      if (m_spacing == NmVector3() || this->m_output == nullptr)
      {
        for(int s=0; s < 3; ++s)
        {
          m_spacing[s] = image->GetSpacing()[s];
          itkSpacing[i] = m_spacing[i];
        }

        if (this->m_output)
        {
          this->m_output->setSpacing(m_spacing);
        }
      } else
      {
        image->SetSpacing(itkSpacing);
      }

      setBlock(image, true);

      ++i;
      blockFile = storage->absoluteFilePath(prefix + multiBlockPath(i));
      dataFetched = true;
    }

    m_bounds = m_blocks_bounding_box;

    return dataFetched && !error;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::snapshot(TemporalStorageSPtr storage, const QString& prefix) const
  {
    using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;
    Snapshot snapshot;

    auto compactedBounds = compactedBlocks();

    for(int i = 0; i < compactedBounds.size(); ++i)
    {
      VolumeWriter::Pointer writer = VolumeWriter::New();

      storage->makePath(prefix);

      // TODO: how to avoid output reference?
      int id = this->m_output?this->m_output->id():0;
      QString name = prefix;
      name += QString("%1_%2_%3").arg(this->type()).arg(id).arg(i);

      QString mhd = name + ".mhd";
      QString raw = name + ".raw";

      auto volume = itkImage(compactedBounds[i].bounds());
      bool releaseFlag = volume->GetReleaseDataFlag();
      volume->ReleaseDataFlagOff();
      writer->SetFileName(storage->absoluteFilePath(mhd).toUtf8().data());
      writer->SetInput(volume);
      writer->Write();
      volume->SetReleaseDataFlag(releaseFlag);

      snapshot << SnapshotData(mhd, storage->snapshot(mhd));
      snapshot << SnapshotData(raw, storage->snapshot(raw));
    }

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::updateBlocksBoundingBox(const VolumeBounds &bounds)
  {
    if (m_blocks_bounding_box.areValid())
    {
      m_blocks_bounding_box = boundingBox(m_blocks_bounding_box, bounds);
    }
    else
    {
      m_blocks_bounding_box = bounds;
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::undo()
  {
    m_blocks.pop_back();
    Q_ASSERT(!m_blocks.empty());

    VolumeBounds bounds = m_blocks[0]->bounds();

    for (auto block: m_blocks)
      bounds = boundingBox(bounds, block->bounds());

    m_blocks_bounding_box = bounds;
    this->updateModificationTime();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBoundsList SparseVolume<T>::boundsPartition(int intervalSize) const
  {
    using SplitBounds = QPair<VolumeBounds, int>;

    NmVector3 minSize{intervalSize*m_spacing[0], intervalSize*m_spacing[1], intervalSize*m_spacing[2]};

    QList<SplitBounds> remaining;
    remaining << SplitBounds(m_bounds, 0);

    VolumeBoundsList blockBounds;

    while(!remaining.isEmpty())
    {
      SplitBounds splitBounds = remaining.takeFirst();
      VolumeBounds bounds = splitBounds.first;

      bool emptyBounds = true;
      int i = 0;
      while (emptyBounds && i < m_blocks.size())
      {
        VolumeBounds blockBounds = m_blocks[i]->bounds();
        if (intersect(blockBounds, bounds))
        {
          emptyBounds = false;
        }

        ++i;
      }

      if (!emptyBounds)
      {
        bool minimumBlockSize = bounds.lenght(Axis::X) <= minSize[0]
                             && bounds.lenght(Axis::Y) <= minSize[1]
                             && bounds.lenght(Axis::Z) <= minSize[2];

        bool needSplit = !minimumBlockSize;

        if (needSplit)
        {
          int splitPlane = splitBounds.second;

          while (bounds.lenght(toAxis(splitPlane)) <= minSize[splitPlane])
          {
            splitPlane = (splitPlane + 1) % 3;
          }

          VolumeBounds b1{bounds.bounds(), bounds.spacing(), bounds.origin()};
          VolumeBounds b2{bounds.bounds(), bounds.spacing(), bounds.origin()};

          Nm splitPoint = (bounds[2*splitPlane] + bounds[2*splitPlane+1]) / 2.0;

          b1.exclude(2*splitPlane+1, splitPoint);
          b2.exclude(2*splitPlane,   splitPoint-bounds.spacing()[splitPlane]);

          splitPlane = (splitPlane + 1) % 3;

          remaining << SplitBounds(b1, splitPlane) << SplitBounds(b2, splitPlane);
        }
        else
        {
          blockBounds << bounds;
        }
      }
    }

    return blockBounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool SparseVolume<T>::isEmpty() const
  {
    if(!isValid())
      return true;

    auto splittedBounds = boundsPartition(50);
    typename T::Pointer image;

    for(auto bounds : splittedBounds)
    {
      image = itkImage(bounds.bounds());

      itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
      it.GoToBegin();
      while (!it.IsAtEnd())
      {
        if(it.Get() != this->backgroundValue())
          return false;

        ++it;
      }
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBoundsList SparseVolume<T>::compactedBlocks() const
  {
    if (m_blocks.empty())
    {
      return VolumeBoundsList();
    }
    // if the last block hasn't been modified then return without doing the compact
    // procedure because we assume the blocks have been compacted before.
    if (m_blocks[m_blocks.size()-1]->isLocked())
    {
      VolumeBoundsList currentBounds;

      for (auto block : m_blocks)
      {
        currentBounds << block->bounds();
      }

      return currentBounds;
    }

    VolumeBoundsList blockBounds = boundsPartition(50);

    VolumeBoundsList nonEmptyBounds;
//    int i = 0;
    for(auto bounds : blockBounds)
    {
//      cout << "Compacting Sparse Volume: " << ++i << "/" << blockBounds.size() << " - " << bounds << endl;
      typename T::Pointer image = itkImage(bounds.bounds());
      bool empty = true;
      itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
      it.GoToBegin();
      while (empty && !it.IsAtEnd())
      {
        empty = it.Get() == this->backgroundValue();
        ++it;
      }

      if (!empty)
      {
        nonEmptyBounds << bounds;
      }
      // blockImages << itkImage(bounds);
    }

    blockBounds.clear();
    blockBounds = nonEmptyBounds;

    // try to join adjacent blocks into a larger ones to reduce the number of blocks.
    // For two blocks to be joinable two out of three coordinates must be the same, and the
    // other one must be adjacent on either side of the direction.
    bool joinable = true;
//    int initialSize = blockBounds.size();

    int pass = 0;
    while(joinable)
    {
      VolumeBoundsList joinedBlocks;
      VolumeBoundsList joinedElements;
      joinable = false;
      for (int i = 0; i < blockBounds.size(); ++i)
        for (int j = 0; j < blockBounds.size(); ++j)
        {
          if (i == j)
          {
            continue;
          }

          if (areAdjacent(blockBounds[i], blockBounds[j]))
          {
            joinable = true;
            if (!joinedElements.contains(blockBounds[i]) && !joinedElements.contains(blockBounds[j]))
            {
              auto joinedBlock = boundingBox(blockBounds[i], blockBounds[j]);
              joinedBlocks << joinedBlock;
              joinedElements << blockBounds[i];
              joinedElements << blockBounds[j];
//              std::cout << "pass " << pass << ": join pair " << i << "-" << j << " -> " << joinedBlock << std::endl;
            }
          }
        }

      for(auto element: joinedElements)
      {
        blockBounds.removeOne(element);
      }
      for(auto joined: joinedBlocks)
      {
        blockBounds << joined;
      }

      joinedBlocks.clear();
      joinedElements.clear();
      ++pass;
    }

//    int finalSize = blockBounds.size();
//    std::cout << initialSize << " initial blocks reduced to " << finalSize << " blocks -> " << 100 - (finalSize *100 / initialSize) << "% reduction." << std::endl;
    for (int i = 0; i < blockBounds.size(); ++i)
    {
      for (int j = i+1; j < blockBounds.size(); ++j)
      {
        Q_ASSERT(!intersect(blockBounds[i], blockBounds[j]));
      }
    }

    return blockBounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T> * sparseCopy(typename T::Pointer image)
  {
    auto bounds  = equivalentBounds<T>(image, image->GetLargestPossibleRegion());
    auto spacing = ToNmVector3<T>(image->GetSpacing());
    auto origin  = ToNmVector3<T>(image->GetOrigin());

    auto sparse = new SparseVolume<T>(bounds, spacing, origin);

    sparse->draw(image, bounds);

    return sparse;
  }

  using SparseVolumePtr  = SparseVolume<itkVolumeType> *;
  using SparseVolumeSPtr = std::shared_ptr<SparseVolume<itkVolumeType>>;
}

#endif // ESPINA_SPARSE_VOLUME_H