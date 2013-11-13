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
    // bounds must be completely inside m_bounds, no partial bounds allowed
    if (!intersect(m_bounds, bounds) || (intersection(m_bounds, bounds) != bounds))
      throw Bounds_Not_Inside_Mask_Exception();

    itkVolumeType::Pointer image = itkVolumeType::New();

    itkVolumeType::IndexType index;
    index[0] = bounds[0]/m_spacing[0];
    index[1] = bounds[2]/m_spacing[1];
    index[2] = bounds[4]/m_spacing[2];

    itkVolumeType::SizeType size;
    size[0] = bounds[1]/m_spacing[0] - bounds[0]/m_spacing[0] + 1;
    size[1] = bounds[3]/m_spacing[1] - bounds[2]/m_spacing[1] + 1;
    size[2] = bounds[5]/m_spacing[2] - bounds[4]/m_spacing[2] + 1;

    itkVolumeType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    itkVolumeType::SpacingType spacing;
    spacing[0] = m_spacing[0];
    spacing[1] = m_spacing[1];
    spacing[2] = m_spacing[2];

    itkVolumeType::PointType origin;
    origin[0] = m_origin[0];
    origin[1] = m_origin[1];
    origin[2] = m_origin[2];

    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->SetOrigin(origin);
    image->Allocate();

    itk::ImageRegionIterator<itkVolumeType> iit(image, region);

    // TODO: pending sparse mask iterators

    return image;
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
          if (!bounds.areValid())
          {
            bounds[0] = bounds[1] = index.x * m_spacing[0];
            bounds[2] = bounds[3] = index.y * m_spacing[1];
            bounds[4] = bounds[5] = index.z * m_spacing[2];
          }
          else
          {
            bounds[0] = std::min(bounds[0], index.x * m_spacing[0]);
            bounds[1] = std::max(bounds[1], index.x * m_spacing[0]);
            bounds[2] = std::min(bounds[2], index.y * m_spacing[1]);
            bounds[3] = std::max(bounds[3], index.y * m_spacing[1]);
            bounds[4] = std::min(bounds[4], index.z * m_spacing[2]);
            bounds[5] = std::max(bounds[5], index.z * m_spacing[2]);
          }
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
    m_blocks_bounding_box[0] = std::min(m_blocks_bounding_box[0], bounds[0]);
    m_blocks_bounding_box[1] = std::max(m_blocks_bounding_box[1], bounds[1]);
    m_blocks_bounding_box[2] = std::min(m_blocks_bounding_box[2], bounds[2]);
    m_blocks_bounding_box[3] = std::max(m_blocks_bounding_box[3], bounds[3]);
    m_blocks_bounding_box[4] = std::min(m_blocks_bounding_box[4], bounds[4]);
    m_blocks_bounding_box[5] = std::max(m_blocks_bounding_box[5], bounds[5]);

    if (!m_blocks_bounding_box.areValid())
      throw Invalid_Internal_State_Exception();
  }

} // namespace EspINA
