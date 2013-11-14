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

#include "Core/Analysis/Data/VolumetricData.h"
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include "Core/Utils/BinaryMask.h"

#include <itkImageRegionIterator.h>

namespace EspINA {

  struct Invalid_image_bounds{};

  template class VolumetricData<itk::Image<unsigned char, 3>>;

  /** \brief Volume representation intended to save memory and speed up
   *  edition opertaions
   *
   *  Those voxel which don't belong to any block are assigned the value
   *  defined as background value.
   *
   *  Add operation will replace every voxel with a value different to 
   *  the background value.
   *  Sub operation will 
   */
  template<typename T>
  class SparseVolume
  : public VolumetricData<T>
  {
    using BlockMask     = BinaryMask<unsigned char>;
    using BlockMaskPtr  = BlockMask*;
    using BlockMaskUPtr = std::unique_ptr<BlockMask>;

  public:
    explicit SparseVolume();
    virtual ~SparseVolume() {}

    explicit SparseVolume(const Bounds& bounds);

    virtual double memoryUsage() const;

    virtual Bounds bounds() const;

    virtual void setOrigin(const typename T::PointType origin);

    virtual typename T::PointType origin() const
    { return m_origin; }

    virtual void setSpacing(const typename T::SpacingType spacing);

    virtual typename T::SpacingType spacing() const
    { return m_spacing; }

    virtual const typename T::Pointer itkImage() const;

    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value);

    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds = Bounds()){}

    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE){}


    virtual void fitToContent(){}

    virtual void resize(const Bounds &bounds);

    virtual void undo() {}

    virtual bool isValid() const{ return false; }

    virtual Snapshot snapshot() const{ return Snapshot(); }

    virtual Snapshot editedRegionsSnapshot() const { return Snapshot(); }

  protected:
    /** \brief Replace sparse volume voxels within data region with data voxels
     *
     * Sparse Volume will take ownership of the block
     */
    void setBlock(typename T::Pointer image);

    /** \brief Set non background data voxels of sparse volume to data voxel values
     *
     * For every voxel in data, set the value of its equivalent sparse volume voxel to
     * the value of the data voxel
     * Sparse Volume will take ownership of the block
     */
    void addBlock(BlockMaskUPtr mask);

    /** \brief Set non background data voxels of sparse volume to background value
     *
     * For every voxel in data, set the value of its equivalent sparse volume voxel to
     * the background value
     * Sparse Volume will take ownership of the block
     */
    void delBlock(BlockMaskUPtr mask);

  private:
    class Block {
    public:
      virtual ~Block(){}

      virtual Bounds bounds() const = 0;
    };

    class AddBlock 
    : public Block {
    public:
      AddBlock(BlockMaskUPtr mask) : m_mask{mask} {}

      Bounds bounds() const { return m_mask->bounds(); }
    private:
      BlockMaskUPtr m_mask;
    };

    class DelBlock 
    : public Block {
    public:
      DelBlock(BlockMaskUPtr mask) : m_mask{mask} {}

      Bounds bounds() const { return m_mask->bounds(); }
    private:
      BlockMaskUPtr m_mask;
    };

    template<typename BT>
    class SetBlock 
    : public Block {
    public:
      SetBlock(typename BT::Pointer image) 
      : m_image{image} {}

      Bounds bounds() const 
      { return equivalentBounds<BT>(m_image, m_image->GetLargestPossibleRegion()); }

    private:
      typename BT::Pointer m_image;
    };

    using ImageIterator = itk::ImageRegionIterator<T>;

  private:
    // TODO: Movable
    BlockMaskUPtr createMask(const Bounds& bounds) const;
// 
//     bool updatePixel(const BlockType op, ImageIterator bit, ImageIterator itt) const;
// 
//     void updateCommonPixels(const BlockType op, const Bounds& bounds, const ImageType::Pointer block, ImageType::Pointer image, ImageType::Pointer mask, int& remainingPixels) const;

    void updateBlocksBoundingBox(const Bounds& bounds);


  private:
    typename T::PointType   m_origin;
    typename T::SpacingType m_spacing;

    using BlockUPtr = std::unique_ptr<Block>;
    using BlockList = std::vector<BlockUPtr>;

    BlockList m_blocks;
    Bounds    m_bounds;
    Bounds    m_blocks_bounding_box;
  };
}

#include "SparseVolume.cpp"

#endif // ESPINA_SPARSE_VOLUME_H
