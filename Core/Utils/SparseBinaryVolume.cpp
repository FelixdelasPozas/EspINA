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

#include "SparseBinaryVolume.h"

#include <vtkImplicitFunction.h>

#include <itkImageRegionIterator.h>
#include <itkImage.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  SparseBinaryVolume::SparseBinaryVolume(const Bounds& bounds, const NmVector3 spacing) throw(Invalid_Image_Bounds_Exception)
  : m_spacing(spacing)
  , m_bounds(bounds)
  , m_blocks_bounding_box(bounds)
  {
    if (!m_bounds.areValid())
      throw Invalid_Image_Bounds_Exception();

    setBlock(BlockMaskUPtr(new BinaryMask<unsigned char>(m_bounds, m_spacing)));
  }

  //----------------------------------------------------------------------------
  SparseBinaryVolume::SparseBinaryVolume(const vtkSmartPointer<vtkImageData> image,
                                         const unsigned char backgroundValue) throw(Invalid_Image_Bounds_Exception)
  {
    double bounds[6], spacing[3];
    image->GetSpacing(spacing);
    image->GetBounds(bounds);

    m_spacing = NmVector3{spacing[0], spacing[1], spacing[2]};
    m_bounds = Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
    m_blocks_bounding_box = m_bounds;

    if (!m_bounds.areValid())
      throw Invalid_Image_Bounds_Exception();

    int scalarSize = image->GetScalarSize();
    unsigned char *pointer = reinterpret_cast<unsigned char *>(image->GetScalarPointer());

    BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(m_bounds, m_spacing);
    BinaryMask<unsigned char>::iterator it(mask);

    Q_ASSERT(static_cast<int>(mask->numberOfVoxels()) == image->GetNumberOfPoints());

    for (auto i = 0; i < image->GetNumberOfPoints(); ++i, ++it)
    {
      if (*pointer != backgroundValue)
        it.Set();

      pointer += scalarSize;
    }

    setBlock(BlockMaskUPtr(mask));
  }

  //----------------------------------------------------------------------------
  template <class T> SparseBinaryVolume::SparseBinaryVolume(const typename T::Pointer image,
                                                            const unsigned char backgroundValue)
  {
    typename T::RegionType region = image->GetLargestPossibleRegion();
    typename T::RegionType::IndexType index = region.GetIndex();
    typename T::RegionType::SizeType size = region.GetSize();
    typename T::SpacingType spacing = image->GetSpacing();

    m_spacing = NmVector3{spacing[0], spacing[1], spacing[2]};
    m_bounds = Bounds{index[0]*spacing[0], (index[0]+size[0]) * spacing[0],
                      index[1]*spacing[1], (index[1]+size[1]) * spacing[1],
                      index[2]*spacing[2], (index[2]+size[2]) * spacing[2]};
    m_blocks_bounding_box = m_bounds;

    if (!m_bounds.areValid())
      throw Invalid_Image_Bounds_Exception();

    using BMask = BinaryMask<unsigned char>;

    BMask *mask = new BMask(m_bounds, m_spacing);
    BMask::iterator mit(mask);

    itk::ImageRegionIterator<T> iit(image, region);

    for (auto i = 0; i < size[0]*size[1]*size[2]; ++i, ++mit, ++iit)
    {
      if (iit.Get() != backgroundValue)
        mit.Set();

    }

    setBlock(BlockMaskUPtr(mask));
  }

  //----------------------------------------------------------------------------
  double SparseBinaryVolume::memoryUsage() const
  {
    unsigned long long bytes = 0;
    for (unsigned int i = 0; i < m_blocks.size(); ++i)
      bytes += m_blocks[i]->memoryUsage();

    return bytes/1048576.; // 1048576 = 1024 * 1024
  }

  //----------------------------------------------------------------------------
  Bounds SparseBinaryVolume::bounds() const
  {
    return m_bounds;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::setOrigin(const NmVector3 &origin)
  {
    m_origin = origin;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::setSpacing(const NmVector3 &spacing)
  {
    m_spacing = spacing;

    m_bounds = Bounds{m_bounds[0] * m_spacing[0], m_bounds[1]*m_spacing[0],
                      m_bounds[2] * m_spacing[1], m_bounds[3]*m_spacing[1],
                      m_bounds[4] * m_spacing[2], m_bounds[5]*m_spacing[2]};

    m_blocks_bounding_box = Bounds{m_blocks_bounding_box[0] * m_spacing[0], m_blocks_bounding_box[1]*m_spacing[0],
                                   m_blocks_bounding_box[2] * m_spacing[1], m_blocks_bounding_box[3]*m_spacing[1],
                                   m_blocks_bounding_box[4] * m_spacing[2], m_blocks_bounding_box[5]*m_spacing[2]};
  }

  //----------------------------------------------------------------------------
  itkVolumeType::Pointer SparseBinaryVolume::itkImage() const
  {
    return itkImage(m_bounds);
  }

  //----------------------------------------------------------------------------
  itkVolumeType::Pointer SparseBinaryVolume::itkImage(const Bounds& bounds) const throw(Bounds_Not_Inside_Mask_Exception)
  {
    // bounds must be completely inside m_bounds, no partial-inside/partial-outside
    // bounds allowed
    if (!intersect(m_bounds, bounds) || (intersection(m_bounds, bounds) != bounds))
      throw Bounds_Not_Inside_Mask_Exception();

    BinaryMaskSPtr<unsigned char> mask = computeMask(bounds);

    return mask->itkImage();
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkImageData> SparseBinaryVolume::vtkImage() const
  {
    return vtkImage(m_bounds);
  }

  //----------------------------------------------------------------------------
  vtkSmartPointer<vtkImageData> SparseBinaryVolume::vtkImage(const Bounds& bounds) const throw(Bounds_Not_Inside_Mask_Exception)
  {
    // bounds must be completely inside m_bounds, no partial-inside/partial-outside
    // bounds allowed
    if (!intersect(m_bounds, bounds) || (intersection(m_bounds, bounds) != bounds))
      throw Bounds_Not_Inside_Mask_Exception();

    BinaryMaskSPtr<unsigned char> mask = computeMask(bounds);

    int extent[6]{ static_cast<int>(bounds[0]/m_spacing[0]), static_cast<int>(bounds[1]/m_spacing[0]),
                   static_cast<int>(bounds[2]/m_spacing[1]), static_cast<int>(bounds[3]/m_spacing[1]),
                   static_cast<int>(bounds[4]/m_spacing[2]), static_cast<int>(bounds[5]/m_spacing[2]) };

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(extent);

    vtkInformation *info = image->GetInformation();
    vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
    vtkImageData::SetNumberOfScalarComponents(1, info);
    image->SetInformation(info);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    image->Modified();

    unsigned char *imagePointer = reinterpret_cast<unsigned char*>(image->GetScalarPointer());
    memset(imagePointer, 0, (extent[1]-extent[0] + 1) * (extent[3]-extent[2] + 1) * (extent[5]-extent[4] + 1));

    BinaryMask<unsigned char>::iterator mit(mask.get());

    mit.goToBegin();
    while (!mit.isAtEnd())
    {

      if (mit.isSet())
        *imagePointer = SEG_VOXEL_VALUE;
      ++imagePointer;
      ++mit;
    }

    return image;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::expandAndDraw(const vtkImplicitFunction*  brush,
                                         const Bounds&               bounds,
                                         const unsigned char         drawValue)
  {
    Bounds intersectionBounds = intersection(bounds, m_bounds);
    if (bounds != intersectionBounds)
      m_bounds = boundingBox(m_bounds, intersectionBounds);

    draw(brush, bounds, drawValue);
  }

  //----------------------------------------------------------------------------
  template <class T> void SparseBinaryVolume::expandAndDraw(const typename T::Pointer   image,
                                                            const Bounds&               bounds,
                                                            const typename T::PixelType backgroundValue)
  {
      Bounds intersectionBounds = intersection(bounds, m_bounds);
      if (bounds != intersectionBounds)
        m_bounds = boundingBox(m_bounds, intersectionBounds);

      draw(image, bounds, backgroundValue);
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::expandAndDraw(const NmVector3 &index,
                                         const bool value)
  {
    Bounds bounds{index[0], index[0], index[1], index[1], index[2], index[2]};

    Bounds intersectionBounds = intersection(bounds, m_bounds);
    if (bounds != intersectionBounds)
      m_bounds = boundingBox(m_bounds, intersectionBounds);

    draw(index, value);
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::draw(const vtkImplicitFunction*  brush,
                                const Bounds&               bounds,
                                const unsigned char         drawValue)
  {
    if (!intersect(bounds, m_bounds))
      return;

    Bounds intersectionBounds = intersection(bounds, m_bounds);

    BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(intersectionBounds, m_spacing);
    BinaryMask<unsigned char>::region_iterator it(mask, intersectionBounds);
    it.goToBegin();
    while (!it.isAtEnd())
    {
      BinaryMask<unsigned char>::IndexType index = it.getIndex();
      double point[3]{ index.x * m_spacing[0], index.y * m_spacing[1], index.z * m_spacing[2] };
      // negative values are inside the implicit function.
      if (brush->FunctionValue(point) < 0)
        it.Set();

      ++it;
    }

    addBlock(BlockMaskUPtr(mask));
    updateBlocksBoundingBox(bounds);

    if (!m_redoBlocks.empty())
      m_redoBlocks.clear();
  }

  //----------------------------------------------------------------------------
  template <class T> void SparseBinaryVolume::draw(const typename T::Pointer image,
                                                   const Bounds&             bounds,
                                                   const typename T::PixelType backgroundValue)
  {
    typename T::SpacingType spacing = image->GetSpacing();
    if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != spacing[2])
      throw Interpolation_Needed_Exception();

    typename T::RegionType region = image->GetLargestPossibleRegion();
    typename T::RegionType::IndexType index = region.GetIndex();
    typename T::RegionType::SizeType size = region.GetSize();

    Bounds imageBounds{ index[0] * spacing[0], (index[0]+size[0]) * spacing[0],
                        index[1] * spacing[1], (index[1]+size[1]) * spacing[1],
                        index[2] * spacing[2], (index[2]+size[2]) * spacing[2]};

    if (!intersect(imageBounds, m_bounds))
      return;

    Bounds intersectionBounds = intersection(imageBounds, m_bounds);
    index[0] = intersectionBounds[0] / spacing[0];
    index[1] = intersectionBounds[2] / spacing[1];
    index[2] = intersectionBounds[4] / spacing[2];
    size[0] = (intersectionBounds[1]-intersectionBounds[0])/spacing[0] + 1;
    size[1] = (intersectionBounds[3]-intersectionBounds[2])/spacing[1] + 1;
    size[2] = (intersectionBounds[5]-intersectionBounds[4])/spacing[2] + 1;

    region.SetIndex(index);
    region.SetSize(size);

    itk::ImageRegionIterator<itkVolumeType> iit(image, region);

    BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(intersectionBounds, m_spacing);
    BinaryMask<unsigned char>::iterator mit(mask);

    mit.goToBegin();
    while (!mit.isAtEnd())
    {
      if (iit.Get() != backgroundValue)
        mit.Set();

      ++mit;
      ++iit;
    }

    addBlock(BlockMaskUPtr(mask));
    updateBlocksBoundingBox(bounds);

    if (!m_redoBlocks.empty())
      m_redoBlocks.clear();
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::draw(const NmVector3 &index,
                                const bool value)
  {
    Bounds bounds{ index[0], index[0], index[1], index[1], index[2], index[2]};

    if (!intersect(bounds, m_bounds))
      return;

    BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(bounds, m_spacing);
    BinaryMask<unsigned char>::iterator it(mask);
    it.goToBegin();
    if (value)
      it.Set();

    addBlock(BlockMaskUPtr(mask));
    updateBlocksBoundingBox(bounds);

    if (!m_redoBlocks.empty())
      m_redoBlocks.clear();
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::fitToContent()
  {
    Bounds bounds;
    BinaryMask<unsigned char>::IndexType index;


    for (unsigned int i = 0; i < m_blocks.size(); ++i)
    {
      if (m_blocks[i]->type() == BlockType::Del)
        continue;

      BinaryMask<unsigned char>::const_region_iterator it(m_blocks[i]->const_region_iterator());
      it.goToBegin();
      while (!it.isAtEnd())
      {
        index = it.getIndex();
        if (it.isSet())
        {
          Bounds voxelBounds{ index.x * m_spacing[0], index.x * m_spacing[0],
                              index.y * m_spacing[1], index.y * m_spacing[1],
                              index.z * m_spacing[2],index.z * m_spacing[2]};
          if (!bounds.areValid())
            bounds = voxelBounds;
          else
            bounds = boundingBox(bounds, voxelBounds);
        }
        ++it;
      }
    }

    // Now bounds are the minimum bounds required to hold the image
    for (unsigned int i = 0; i < m_blocks.size(); ++i)
    {
      Bounds intersectionBounds = intersection(bounds, m_blocks[i]->bounds());

      BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(intersectionBounds, m_spacing);
      BinaryMask<unsigned char>::iterator mit(mask);
      BinaryMask<unsigned char>::const_region_iterator bit = m_blocks[i]->const_region_iterator(intersectionBounds);

      mit.goToBegin();
      bit.goToBegin();
      while (!mit.isAtEnd())
      {
        if (bit.isSet())
          mit.Set();

        ++mit;
        ++bit;
      }

      // replace the old (probably bigger) block with the new smaller one.
      m_blocks[i] = std::move(BlockUPtr{ new Block(BlockMaskUPtr(mask), m_blocks[i]->type()) });
    }

    m_blocks_bounding_box = m_bounds = bounds;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::resize(const Bounds &bounds)
  {
    if (!intersect(m_bounds, bounds) || (intersection(m_bounds, bounds) != m_bounds))
      throw Bounds_Reduce_Original_Image_Exception{};

    m_bounds = bounds;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::undo() throw(Cant_Undo_Exception)
  {
    if (m_blocks.back()->isLocked())
      throw Cant_Undo_Exception();

    BlockUPtr block = std::move(m_blocks.back());
    m_blocks.pop_back();

    m_redoBlocks.push_back(std::move(block));
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::redo() throw(Cant_Redo_Exception)
  {
    if (m_redoBlocks.empty())
      throw Cant_Redo_Exception();

    BlockUPtr block = std::move(m_redoBlocks.back());
    m_redoBlocks.pop_back();

    m_blocks.push_back(std::move(block));
  }

  //----------------------------------------------------------------------------
  Snapshot SparseBinaryVolume::snapshot() const
  {
    Snapshot snapshot;
    for (unsigned int i = 0; i < m_blocks.size(); ++i)
      snapshot.push_back(QPair<QString, QByteArray>(QString("Block ") + QString::number(i), m_blocks[i]->byteArray()));

    return snapshot;
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::setBlock(SparseBinaryVolume::BlockMaskUPtr image)
  {
    m_blocks.push_back(BlockUPtr{ new Block(std::move(image), BlockType::Set) });
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::addBlock(SparseBinaryVolume::BlockMaskUPtr mask)
  {
    m_blocks.push_back(BlockUPtr{ new Block(std::move(mask), BlockType::Add) });
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::delBlock(SparseBinaryVolume::BlockMaskUPtr mask)
  {
    m_blocks.push_back(BlockUPtr{ new Block(std::move(mask), BlockType::Del) });
  }

  //----------------------------------------------------------------------------
  SparseBinaryVolume::BlockMaskUPtr SparseBinaryVolume::createMask(const Bounds& bounds) const
  {
    return BlockMaskUPtr{ new BinaryMask<unsigned char>(bounds) };
  }

  //----------------------------------------------------------------------------
  void SparseBinaryVolume::updateBlocksBoundingBox(const Bounds& bounds) throw(Invalid_Internal_State_Exception)
  {
    m_blocks_bounding_box = boundingBox(m_blocks_bounding_box, bounds);

    if (!m_blocks_bounding_box.areValid())
      throw Invalid_Internal_State_Exception();
  }

  //----------------------------------------------------------------------------
  BinaryMaskSPtr<unsigned char> SparseBinaryVolume::computeMask(const Bounds &bounds) const
  {
    auto mask = new BinaryMask<unsigned char>(bounds, m_spacing);
    auto bitsetMask = new BinaryMask<unsigned char>(bounds, m_spacing);
    unsigned long long numVoxels = bitsetMask->numberOfVoxels();

    for (unsigned int i = m_blocks.size() -1; i >= 0; --i)
    {
      if (!intersect(bounds, m_blocks[i]->bounds()))
        continue;

      if (numVoxels == 0)
        break;

      Bounds intersectionBounds = intersection(bounds, m_blocks[i]->bounds());

      BinaryMask<unsigned char>::region_iterator bitsetIt(bitsetMask, intersectionBounds);
      BinaryMask<unsigned char>::region_iterator mri(mask, intersectionBounds);
      BinaryMask<unsigned char>::const_region_iterator bri(m_blocks[i]->const_region_iterator(intersectionBounds));

      mri.goToBegin();
      bri.goToBegin();
      bitsetIt.goToBegin();

      switch(m_blocks[i]->type())
      {
        case BlockType::Set:
          while(!mri.isAtEnd())
          {
            if (!bitsetIt.isSet() && bri.isSet())
              mri.Set();
            else
              mri.Unset();

            bitsetIt.Set();
            --numVoxels;

            ++mri;
            ++bri;
            ++bitsetIt;
          }
          break;
        case BlockType::Add:
          while(!mri.isAtEnd())
          {
            if (!bitsetIt.isSet() && bri.isSet())
            {
              mri.Set();
              bitsetIt.Set();
              --numVoxels;
            }

            ++mri;
            ++bri;
            ++bitsetIt;
          }
          break;
        case BlockType::Del:
          while(!mri.isAtEnd())
          {
            if (!bitsetIt.isSet() && bri.isSet())
            {
              mri.Unset();
              bitsetIt.Set();
              --numVoxels;
            }

            ++mri;
            ++bri;
            ++bitsetIt;
          }
          break;
        default:
          Q_ASSERT(false);
          break;
      }
    }

    return BinaryMaskSPtr<unsigned char>(mask);
  }

} // namespace EspINA
