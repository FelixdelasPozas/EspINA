/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_SPARSE_BINARY_VOLUME_H_
#define ESPINA_SPARSE_BINARY_VOLUME_H_

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include "BinaryMask.h"

// VTK
#include <vtkSmartPointer.h>

class vtkImageData;

namespace EspINA
{
  class SparseBinaryVolume
  {
    struct Invalid_Image_Bounds_Exception{};
    struct Bounds_Not_Inside_Mask_Exception{};
    struct Bounds_Reduce_Original_Image_Exception{};
    struct Cant_Redo_Exception{};
    struct Cant_Undo_Exception{};
    struct Invalid_Internal_State_Exception{};
    struct Interpolation_Needed_Exception{};

  /** \brief Volume representation intended to save memory and speed up
   *  edition operations
   *
   *  Those voxel which don't belong to any block are assigned the value
   *  defined as background value.
   *
   *  Add operation will replace every voxel with a value different to
   *  the background value.
   *  Sub operation will
   */
    using BlockMask     = BinaryMask<unsigned char>;
    using BlockMaskPtr  = BlockMask*;
    using BlockMaskUPtr = std::unique_ptr<BlockMask>;

  public:
    /* \brief SparseBinaryVolume constructor to create a empty mask with the given bounds and spacing.
     */
    explicit SparseBinaryVolume(const Bounds& bounds = Bounds(),
                                const NmVector3 spacing = NmVector3{1,1,1}) throw(Invalid_Image_Bounds_Exception);

    /* \brief SparseBinaryVolume constructor to create the equivalent mask from a vtkImageData.
     */
    explicit SparseBinaryVolume(const vtkSmartPointer<vtkImageData> image,
                                const unsigned char backgroundValue = 0) throw(Invalid_Image_Bounds_Exception);

    /* \brief SparseBinaryVolume constructor to create the equivalent mask from a templated itk::Image.
     */
    template <class T> explicit SparseBinaryVolume(const typename T::Pointer image,
                                                   const unsigned char backgroundValue = 0);

    /* \brief Class destructor
     */
    ~SparseBinaryVolume() {};

    /* \brief Returns the memory used to store the image in megabytes.
     */
    double memoryUsage() const;

    /* \brief Returns the bounds of the mask, meaning its the smallest bounds that contains all the
     * blocks of the mask.
     */
    Bounds bounds() const;

    /* \brief Set the origin of the mask.
     */
    void setOrigin(const NmVector3 &origin);

    /* \brief Returns the origin of the mask.
     */
    NmVector3 origin() const
    { return m_origin; }

    /* \brief Returns the spacing of the mask.
     */
    void setSpacing(const NmVector3 &spacing);

    NmVector3 spacing() const
    { return m_spacing; }

    /* \brief Return the mask image as a itkVolumeType ( = itk::Image<unsigned char, 3>).
     */
    itkVolumeType::Pointer itkImage() const;

    /* \brief Return a region of the mask image as a itkVolumeType ( = itk::Image<unsigned char, 3>).
     */
    itkVolumeType::Pointer itkImage(const Bounds& bounds) const throw(Bounds_Not_Inside_Mask_Exception);

    /* \brief Return the mask image as a vtkImageData of unsigned char scalars.
     */
    vtkSmartPointer<vtkImageData> vtkImage() const;

    /* \brief Return a region of the mask image as a vtkImageData of unsigned char scalars.
     */
    vtkSmartPointer<vtkImageData> vtkImage(const Bounds& bounds) const throw(Bounds_Not_Inside_Mask_Exception);

    /* \brief Draw method to modify mask using a implicit function.
     */
    void draw(const vtkImplicitFunction*  brush,
              const Bounds&               bounds,
              const unsigned char         drawValue);

    /* \brief Draw method to modify mask using a templated itk::Image.
     */
    template <class T> void draw(const typename T::Pointer   image,
                                 const Bounds&               bounds = Bounds(),
                                 const typename T::PixelType backgroundValue = 0);

    /* \brief Draw method to modify a voxel of the mask.
     */
    void draw(const NmVector3 &index,
              const bool value = true);

    /* \brief Draw method to expand the image bounds (only if necessary) and modify
     * the mask using a implicit function.
     */
    void expandAndDraw(const vtkImplicitFunction*  brush,
                       const Bounds&               bounds,
                       const unsigned char         drawValue);

    /* \brief Draw method to expand the image bounds (only if necessary) and modify
     * the mask using a templated itk::Image.
     */
    template <class T> void expandAndDraw(const typename T::Pointer   image,
                                          const Bounds&               bounds = Bounds(),
                                          const typename T::PixelType backgroundValue = 0);

    /* \brief Draw method to expand the image bounds (only if necessary) and modify
     *  a voxel of the mask.
     */
    void expandAndDraw(const NmVector3 &index,
                       const bool value = true);

    /* \brief Reduce the bounds to the smallest one that contains the mask (clipping empty borders).
     */
    void fitToContent();

    /* \brief Change the bounds of the image.
     */
    void resize(const Bounds &bounds);

    /* \brief Undo interface to revert the changes made by the draw methods to the mask.
     */
    void undo() throw(Cant_Undo_Exception);

    /* \brief Redo interface to revert the changes made by the undo method to the mask.
     */
    void redo() throw(Cant_Redo_Exception);

    /* \brief Persistent Interface to save the mask state.
     */
    Snapshot snapshot() const;

  protected:
    /* \brief Replace mask voxels within data region with new values.
     * Sparse Volume will take ownership of the block.
     */
    void setBlock(BlockMaskUPtr image);

    /* \brief Set mask voxels of the region to set value.
     * Sparse Volume will take ownership of the block.
     */
    void addBlock(BlockMaskUPtr mask);

    /* \brief Set mask voxels of the region to unset value.
     * Sparse Volume will take ownership of the block.
     */
    void delBlock(BlockMaskUPtr mask);

    /* \brief Compute a mask from a given size
     *
     */
    BinaryMaskSPtr<unsigned char> computeMask(const Bounds &bounds) const;

  private:
    enum class BlockType
    { Set = 0, Add = 1, Del = 2 };

    class Block
    {
    public:
      /* \brief Block class constructor.
       * mask: BinaryMask<unsigned char> unique pointer
       * type: Set, Add, Del
       * locked: Moveable to redo list.
       */
      explicit Block(BlockMaskUPtr mask, BlockType type = BlockType::Set, bool locked = false)
      : m_mask{std::move(mask)}
      , m_type(type)
      , m_locked(locked)
      {}

      /* \brief Block class destructor.
       */
      virtual ~Block(){}

      /* \brief Returns if this block can be moved to redo block list.
       */
      bool isLocked()
      { return m_locked; }

      /* \brief Returns type of the block.
       */
      BlockType type()
      { return m_type; }

      /* \brief Returns the contents of the mask buffer as a QByteArray.
       */
      QByteArray byteArray()
      { return m_mask->buffer(); }

      /* \brief Returns the memory used for data storage in bytes.
       */
      unsigned long memoryUsage()
      { return m_mask->bufferSize(); }

      Bounds bounds() const
      { return m_mask->bounds(); }

      BinaryMask<unsigned char>::const_region_iterator const_region_iterator(const Bounds &bounds = Bounds()) const
      {
        if (!bounds.areValid())
          bounds = m_mask->bounds();

        BinaryMask<unsigned char>::const_region_iterator it(m_mask.get(), bounds);
        return it;
      }

    private:
      BlockMaskUPtr m_mask;
      BlockType     m_type;
      bool          m_locked;

    };

  private:
    /* \brief Class internal method to create an empty mask.
     */
    BlockMaskUPtr createMask(const Bounds& bounds) const;

    /* \brief Class internal method to compute bounding box
     */
    void updateBlocksBoundingBox(const Bounds& bounds) throw(Invalid_Internal_State_Exception);

  private:
    NmVector3 m_origin;
    NmVector3 m_spacing;

    using BlockUPtr = std::unique_ptr<Block>;
    using BlockList = std::vector<BlockUPtr>;

    BlockList    m_blocks;
    BlockList    m_redoBlocks;
    Bounds       m_bounds;
    Bounds       m_blocks_bounding_box;
  };
} // namespace EspINA

#endif // ESPINA_SPARSE_BINARY_VOLUME_H_
