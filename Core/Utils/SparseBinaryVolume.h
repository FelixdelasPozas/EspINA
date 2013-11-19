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

  class SparseBinaryVolume;
  using SparseBinaryVolumePtr  = SparseBinaryVolume *;
  using SparseBinaryVolumeSPtr = std::shared_ptr<SparseBinaryVolume>;

  class SparseBinaryVolume
  {
    struct Invalid_Image_Bounds_Exception{};
    struct Bounds_Not_Inside_Mask_Exception{};
    struct Bounds_Reduce_Original_Image_Exception{};
    struct Cant_Redo_Exception{};
    struct Cant_Undo_Exception{};
    struct Invalid_Internal_State_Exception{};
    struct Interpolation_Needed_Exception{};

    // iterator exceptions (to match with BinaryMask::iterators exceptions
    struct Out_Of_Bounds_Exception{};
    struct Underflow_Exception{};
    struct Overflow_Exception{};
    struct Const_Violation_Exception{};
    struct Region_Not_Contained_In_Mask_Exception{};

  /** \brief Volume representation intended to save memory and speed up
   *  edition operations
   *
   *  Those voxel which don't belong to any block are assigned the value
   *  defined as background value.
   *
   *  Add operation will replace every voxel with a value different to
   *  the background value.
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

    /* \brief Returns the bounds of the mask.
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

  public:
    //- ITERATOR CLASS --------------------------------------------------------------------
    class iterator
    : public std::iterator<std::bidirectional_iterator_tag, unsigned char>
    {
      public:
        /** \brief iterator for a given mask constructor.
         * NOTE: it creates a mask for the whole region to iterate. In this case, the whole image.
         */
        iterator(SparseBinaryVolumeSPtr mask)
        : m_it(mask->computeMask(mask->bounds()).get(), mask->bounds())
        , m_mask(mask)
        {
        }

        /** \brief Constructor for begin() and end() iterators, and others.
         */
        iterator(const iterator& it, const unsigned long long pos = 0, const int bitPos = 0)
        : m_it(it.m_it)
        , m_mask(it.m_mask)
        {
        }

        virtual ~iterator() {};

        /** \brief Returns an iterator positioned at the beginning of the mask
         *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
         *        is not the intended here.
         */
        iterator begin()
        {
          return iterator(*this);
        }

        /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
         *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
         *        is not the intended here.
         *  NOTE: Trying to get or set a value of this iterator position will throw an
         *        exception.
         */
        iterator end()
        {
          m_it.goToEnd();

          return *this;
        }

        /** \brief Sets the position of the iterator to the beginning of the mask.
         */
        void goToBegin()
        {
          m_it.goToBegin();
        }

        /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
         */
        void goToEnd()
        {
          m_it.goToEnd();
        }

        /** \brief Returns true if the iterator is positioned at the end of the mask.
         */
        bool isAtEnd() const
        {
          return (m_it.isAtEnd());
        }

        /** \brief Equal operator between iterator and an iterator or a const_iterator of the mask.
         */
        bool operator==(const iterator& other) const
        {
          return (m_it == other.m_it);
        }

        /** \brief Non equal operator between iterator and an iterator or a const_iterator of the mask.
         */
        bool operator!=(const iterator& other) const
        {
          return (m_it != other.m_it);
        }

        /** \brief Returns the value at the position of the iterator.
         *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
         *  one-past-the-end element of the mask.
         */
        bool Get() const throw(Out_Of_Bounds_Exception)
        {
          if (isAtEnd())
            throw Out_Of_Bounds_Exception();

          if (SEG_BG_VALUE != m_it.Get())
            return true;

          return false;
        }

        /* \brief Convenience method when one doesn't need to know the foreground/background
         * values of the mask.
         */
        bool isSet()
        {
          return (m_it.Get() == SEG_VOXEL_VALUE);
        }

        /** \brief Sets the value of the pointed element of the mask to foreground value.
         *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
         *  one-past-the-end element of the mask.
         */
        void Set() throw(Out_Of_Bounds_Exception)
        {
          if (isAtEnd())
            throw Out_Of_Bounds_Exception();

          BinaryMask<unsigned char>::IndexType index = m_it.getIndex();
          NmVector3 spacing = m_mask->spacing();
          NmVector3 point{ index.x * spacing[0], index.y * spacing[1], index.z * spacing[2] };
          m_mask->draw(point, true);
        }

        /** \brief Sets the value of the pointed element of the mask to background value.
         *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
         *  one-past-the-end element of the mask.
         */
        void Unset() throw(Out_Of_Bounds_Exception)
        {
          if (isAtEnd())
            throw Out_Of_Bounds_Exception();

          BinaryMask<unsigned char>::IndexType index = m_it.getIndex();
          NmVector3 spacing = m_mask->spacing();
          NmVector3 point{ index.x * spacing[0], index.y * spacing[1], index.z * spacing[2] };
          m_mask->draw(point, false);
        }

        /** \brief Decrements the iterator position.
         *  NOTE: Can throw an Underflow_Exception if the iterator is positioned at
         *  the beginning of the mask.
         */
        iterator& operator--() throw(Underflow_Exception)
        {
          --m_it;
          return *this;
        }

        /** \brief Increments the iterator position.
         *  NOTE: Can throw an Overflow_Exception if the iterator is positioned at
         *  one-past-the-end element of the mask.
         */
        iterator &operator++() throw (Overflow_Exception)
        {
          ++m_it;
          return *this;
        }

      protected:
        /* \brief Constructor for region_iterator version.
         */
        iterator(SparseBinaryVolumeSPtr mask, const Bounds &bounds)
        : m_it(mask->computeMask(bounds).get(), bounds)
        , m_mask(mask)
        {}

        BinaryMask<unsigned char>::region_iterator m_it;
        SparseBinaryVolumeSPtr                     m_mask;

        friend class region_iterator;
    };

    //- CONST ITERATOR CLASS --------------------------------------------------------------
    class const_iterator
    : public iterator
    {
      public:
        /** \brief const_iterator for a given mask constructor
         */
        const_iterator(SparseBinaryVolumeSPtr mask)
        : iterator(mask)
        {
        };

        virtual ~const_iterator() {};

        /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
         */
        void Set() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }

        /** \brief Forbidden Unset() will throw a Const_Violation_Exception exception.
         */
        void Unset() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }
    };

    //- REGION ITERATOR CLASS -------------------------------------------------------------
    class region_iterator
    : public iterator
    {
      public:
        /** \brief region_iterator for a given region of a given mask constructor.
         *  NOTE: Can throw a Region_Not_Contained_In_Mask if the given region is not
         *        inside the largest possible region of the mask.
         */
        // TODO
        region_iterator(SparseBinaryVolumeSPtr mask, const Bounds &bounds) throw (Region_Not_Contained_In_Mask_Exception)
        : iterator(mask, bounds)
        {
          if (bounds != intersection(mask->bounds(), bounds))
            throw Region_Not_Contained_In_Mask_Exception();
        }

        virtual ~region_iterator() {};

      protected:

    };

    //- CONST REGION ITERATOR CLASS -------------------------------------------------------
    class const_region_iterator
    : public region_iterator
    {
      public:
        /** \brief const_region_iterator for a given region of a given mask constructor
         */
        const_region_iterator(SparseBinaryVolumeSPtr mask, const Bounds &bounds)
        : region_iterator(mask, bounds)
        {
        }

        virtual ~const_region_iterator() {};

        /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
         */
        void Set() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }

        /** \brief Forbidden Unet() will throw a Const_Violation_Exception exception.
         */
        void Unset() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }
    };

  };
} // namespace EspINA

#endif // ESPINA_SPARSE_BINARY_VOLUME_H_