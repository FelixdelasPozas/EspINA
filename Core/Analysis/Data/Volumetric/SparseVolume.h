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

#include "EspinaCore_Export.h"

#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Utils/BinaryMask.h>

namespace EspINA {

  struct Invalid_Image_Bounds_Exception{};

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
  class EspinaCore_EXPORT SparseVolume
  : public VolumetricData<T>
  {
    using BlockMask     = BinaryMask<unsigned char>;
    using BlockMaskPtr  = BlockMask*;
    using BlockMaskUPtr = std::unique_ptr<BlockMask>;

  public:
    /** \brief SparseVolume constructor to create a empty mask with the given bounds and spacing.
     */
    explicit SparseVolume(const Bounds& bounds = Bounds(), const NmVector3& spacing = {1, 1, 1}) throw(Invalid_Image_Bounds_Exception);

    /** \brief Class destructor
     */
    virtual ~SparseVolume() {}

    /** \brief Returns the memory used to store the image in megabytes.
     */
    virtual double memoryUsage() const;

    /** \brief Returns the bounds of the volume.
     */
    virtual Bounds bounds() const;

    /** \brief Set volume origin.
     */
    virtual void setOrigin(const NmVector3& origin);

    /** \brief Returns volume origin.
     */
    virtual NmVector3 origin() const
    { return m_origin; }

    /** \brief Set volume spacing.
     */
    virtual void setSpacing(const NmVector3& spacing);

    /** \brief Returns volume spacing.
     */
    virtual NmVector3 spacing() const
    { return m_spacing; }

    /** \brief Returns the equivalent itk image of the volume.
     */
    virtual const typename T::Pointer itkImage() const;

    /** \brief Returns the equivalent itk image of a region of the volume.
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    /** \brief Method to modify the volume using a implicit function.
     *
     *  Draw methods are constrained to volume bounds.
     */
    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value = SEG_VOXEL_VALUE);

    /** \brief Method to modify the volume using an itk image.
     *
     *  Draw methods are constrained to volume bounds.
     */
    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds = Bounds());

    /** \brief Method to modify a voxel of the volume using an itk index.
     *
     *  Draw methods are constrained to volume bounds.
     */
    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE);

    // ------------------------------------------------------------------------
    // TODO: expand and draw
    // TODO: extract region of sparse volume as vtkImageData
    // TODO: iterators?
    // TODO: fitToContent(), undo(), redo() y pila de cambios como en SparseBinary
    // TODO: snapshot(), editedregionSnapshot()
    // TODO: volumen 3d itkVolumeType to SparseVolume (algoritmo de octree)
    // ------------------------------------------------------------------------

    /** \brief Resizes the image to the minimum bounds that can contain the volume.
     *
     *  The resultant image is always smaller of equal in size to the original one.
     */
    virtual void fitToContent(){}

    /** \brief Resize the volume bounds. The given bounds must containt the original.
     */
    virtual void resize(const Bounds &bounds);

    /** \brief Method to undo the last change made to the volume.
     */
    virtual void undo() {}

    /** \brief Returns if the volume has been correctly initialized.
     */
    virtual bool isValid() const;

    /** \brief Try to load data from storage
     *
     *  Return if any data was fetched
     */
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString& prefix);

    /** \brief Persistent Interface to save the mask state.
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const;

    virtual Snapshot editedRegionsSnapshot() const { return Snapshot(); }

    void compact();

  protected:
    /** \brief Replace sparse volume voxels within data region with data voxels
     *
     *  Sparse Volume will take ownership of the block
     */
    void setBlock(typename T::Pointer image);

    /** \brief Set non background data voxels of sparse volume to data voxel values
     *
     *  For every voxel in data, set the value of its equivalent sparse volume voxel to
     *  the value of the data voxel
     *  Sparse Volume will take ownership of the block
     */
    void addBlock(BlockMaskUPtr mask);

  private:
    enum class BlockType
    { Set = 0, Add = 1 };

    class Block;
    using BlockUPtr = std::unique_ptr<Block>;
    using BlockList = std::vector<BlockUPtr>;

    class Block
    {
      public:
        struct Invalid_Iteration_Bounds_Exception{};

      public:
        Block(BlockMaskUPtr mask, bool locked)
        : m_type(BlockType::Add)
        , m_locked(locked)
        , m_mask(std::move(mask))
        , m_image(nullptr)
        {}

        Block(typename T::Pointer image, bool locked)
        : m_type(BlockType::Set)
        , m_locked(locked)
        , m_mask(nullptr)
        , m_image(image)
        {}

        ~Block()
        {}

        Bounds bounds() const
        {
          if (m_image != nullptr)
            return equivalentBounds<T>(m_image, m_image->GetLargestPossibleRegion());
          else
            return m_mask->bounds();
        }

        unsigned long long numberOfVoxels()
        {
          if (m_image != nullptr)
            return m_image->GetBufferedRegion().GetNumberOfPixels();
          else
            return m_mask->numberOfVoxels();
        }

        BlockType type() const
        { return m_type; }

        bool isLocked() const
        { return m_locked; }

      protected:
        friend class SparseVolume;

        BlockType     m_type;
        bool          m_locked;

        BlockMaskUPtr       m_mask;
        typename T::Pointer m_image;
    };

  private:
    BlockMaskUPtr createMask(const Bounds& bounds) const;
    void updateBlocksBoundingBox(const Bounds& bounds);

  private:
    NmVector3 m_origin;
    NmVector3 m_spacing;

    BlockList m_blocks;
    Bounds    m_bounds;
    Bounds    m_blocks_bounding_box;
  };
}
#include "SparseVolume.cpp"


#endif // ESPINA_SPARSE_VOLUME_H
