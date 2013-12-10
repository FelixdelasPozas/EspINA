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

// EspINA
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Persistent.h>

// VTK
#include <vtkImplicitFunction.h>

namespace EspINA
{

  const int UNSET = 0;
  const int SET   = 1;

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing)
  : VolumetricData<T>()
  , m_origin{0,0,0}
  , m_spacing{spacing}
  {
    if (bounds.areValid())
    {
      auto region = equivalentRegion<T>(m_origin, m_spacing, bounds);

      m_bounds = equivalentBounds<T>(m_origin, m_spacing, region);
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
  const Bounds SparseVolume<T>::bounds() const
  {
    return m_bounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setOrigin(const NmVector3& origin)
  {
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

      auto region = equivalentRegion<T>(m_origin, m_spacing, m_bounds);

      m_spacing = spacing;

      m_bounds = equivalentBounds<T>(m_origin, m_spacing, region);
    }
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setBlock(typename T::Pointer image)
  {
    BlockSPtr block(new ImageBlock(image, false));
    m_blocks.push_back(block);

    //Bounds bounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());

    /*// DEBUG
    auto i2 = T::New();
    i2->SetSpacing(image->GetSpacing());
    auto region = equivalentRegion<T>(i2, bounds);

    image->GetLargestPossibleRegion().Print(std::cout);
    region.Print(std::cout);
    */

    updateBlocksBoundingBox(block->bounds());
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::addBlock(BlockMaskSPtr mask)
  {
    Bounds bounds = mask->bounds();

    BlockSPtr block(new AddBlock(mask, false));
    m_blocks.push_back(block);

    updateBlocksBoundingBox(bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage() const
  {
    return itkImage(m_bounds);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer SparseVolume<T>::itkImage(const Bounds& bounds) const
  {
    if (!m_bounds.areValid())
      throw Invalid_Image_Bounds_Exception();

    if (!intersect(m_bounds, bounds) || (bounds != intersection(m_bounds, bounds)))
      throw Invalid_Image_Bounds_Exception();

    auto image       = create_itkImage<T>(bounds, this->backgroundValue(), m_spacing, m_origin);
    auto imageRegion = image->GetLargestPossibleRegion();
    auto maskBounds  = equivalentBounds<T>(image, imageRegion);
    auto mask        = new BinaryMask<unsigned char>(maskBounds, m_spacing);
    auto numVoxels   = imageRegion.GetNumberOfPixels();
    Q_ASSERT(numVoxels == mask->numberOfVoxels());

    for (int i = m_blocks.size() - 1; i >= 0; --i)
    {
      auto block = m_blocks[i];

      Bounds blockBounds = block->bounds();
      if (!intersect(bounds, blockBounds))
        continue;

      if (numVoxels == 0)
        break;

      Bounds intersectionBounds = intersection(maskBounds, blockBounds);

      BinaryMask<unsigned char>::region_iterator mit(mask, intersectionBounds);
      itk::ImageRegionIterator<T> iit(image, equivalentRegion<T>(image, intersectionBounds));
      mit.goToBegin();
      iit = iit.Begin();

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
    if (!intersect(m_bounds, bounds))
      return;

    Bounds intersectionBounds = intersection(m_bounds, bounds);

    BlockMaskSPtr mask{new BinaryMask<unsigned char>(intersectionBounds, m_spacing)};
    mask->setForegroundValue(value);
    BinaryMask<unsigned char>::region_iterator it(mask.get(), intersectionBounds);

    it.goToBegin();
    while (!it.isAtEnd())
    {
      BinaryMask<unsigned char>::IndexType index = it.getIndex();
      const double point[3]{ index.x * m_spacing[0], index.y * m_spacing[1], index.z * m_spacing[2]};
      if (const_cast<vtkImplicitFunction *>(brush)->FunctionValue(point[0], point[1], point[2]) <= 0)
        it.Set();
      ++it;
    }

    addBlock(mask);
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
    addBlock(mask);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer volume,
                             const Bounds&             bounds)
  {
    if (!intersect(m_bounds, bounds))
      return;

    Bounds drawBounds   = intersection(m_bounds, bounds);
    Bounds volumeBounds = equivalentBounds<itkVolumeType>(volume, volume->GetLargestPossibleRegion());

    typename T::Pointer block;

    if (drawBounds != volumeBounds)
    {
      using ExtractorType = itk::ExtractImageFilter<T,T>;

      drawBounds = intersection(drawBounds, volumeBounds);

      auto extractor = ExtractorType::New();
      auto region    = equivalentRegion<T>(volume, drawBounds);

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
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void SparseVolume<T>::draw(const typename T::IndexType index,
                             const typename T::PixelType value)
  {
      Bounds bounds { index[0] * m_spacing[0], index[0] * m_spacing[0],
                      index[1] * m_spacing[1], index[1] * m_spacing[1],
                      index[2] * m_spacing[2], index[2] * m_spacing[2] };

      if (!intersect(m_bounds, bounds))
        return;

      Bounds intersectionBounds = intersection(m_bounds, bounds);
      BlockMaskSPtr mask{new BinaryMask<unsigned char>(intersectionBounds, m_spacing)};
      mask->setForegroundValue(value);
      BinaryMask<unsigned char>::region_iterator it(mask.get(), intersectionBounds);
      it.goToBegin();
      it.Set();

      addBlock(mask);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::resize(const Bounds &bounds)
  {
    m_bounds = bounds;
    m_blocks_bounding_box = intersection(m_bounds, m_blocks_bounding_box);

    // TODO: Reduce existing blocks??
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
    QFileInfo blockFile(storage->absoluteFilePath(prefix + QString("VolumetricData_%1.mhd").arg(i)));

    m_spacing = this->m_output->spacing();

    auto itkSpacing = ItkSpacing<T>(m_spacing);

    while (blockFile.exists())
    {
      VolumeReader::Pointer reader = VolumeReader::New();
      reader->SetFileName(blockFile.absoluteFilePath().toUtf8().data());
      reader->Update();

      auto image = reader->GetOutput();

      if (m_spacing == NmVector3())
      {
        for(int s=0; s < 3; ++s)
        {
          m_spacing[s] = image->GetSpacing()[s];
          itkSpacing[i] = m_spacing[i];
        }
      } else
      {
        image->SetSpacing(itkSpacing);
      }

      setBlock(image);

      ++i;
      blockFile = storage->absoluteFilePath(prefix + QString("VolumetricData_%1.mhd").arg(i));
      dataFetched = true;
    }

    m_bounds  = m_blocks_bounding_box;

    return dataFetched && !error;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot SparseVolume<T>::snapshot(TemporalStorageSPtr storage, const QString& prefix) const
  {
    using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;
    Snapshot snapshot;

    compact();

    for(int i = 0; i < m_blocks.size(); ++i)
    {
      VolumeWriter::Pointer writer = VolumeWriter::New();

      storage->makePath(prefix);

      QString name = prefix;
      name += QString("%1_%2").arg(this->type()).arg(i);

      QString mhd = name + ".mhd";
      QString raw = name + ".raw";

      ImageBlock * block = dynamic_cast<ImageBlock *>(m_blocks[i].get());
      Q_ASSERT(block);

      if (block)
      {
        auto volume = block->m_image;
        bool releaseFlag = volume->GetReleaseDataFlag();
        volume->ReleaseDataFlagOff();
        writer->SetFileName(storage->absoluteFilePath(mhd).toUtf8().data());
        writer->SetInput(volume);
        writer->Write();
        volume->SetReleaseDataFlag(releaseFlag);

        snapshot << SnapshotData(mhd, storage->snapshot(mhd));
        snapshot << SnapshotData(raw, storage->snapshot(raw));
      }
    }

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::updateBlocksBoundingBox(const Bounds& bounds)
  {
    if (m_blocks_bounding_box.areValid()) {
      m_blocks_bounding_box = boundingBox(m_blocks_bounding_box, bounds);
    } else {
      m_blocks_bounding_box = bounds;
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::undo()
  {
    m_blocks.pop_back();
    Q_ASSERT(!m_blocks.empty());

    Bounds bounds = m_blocks[0]->bounds();
    for (auto block: m_blocks)
      bounds = boundingBox(bounds, block->bounds());

    m_blocks_bounding_box = bounds;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::compact()
  {
    using SplitBounds = QPair<Bounds, int>;

    int minSize[3] = {50, 50, 50};
    for(int i = 0; i < 3; ++i)
    {
      minSize[i] *= m_spacing[i];
    }

    QList<SplitBounds> remaining;
    remaining << SplitBounds(m_blocks_bounding_box, 0);

    QList<Bounds> blockBounds;

    while(!remaining.isEmpty())
    {
      SplitBounds splitBounds = remaining.takeFirst();
      Bounds bounds = splitBounds.first;

      bool emptyBounds = true;
      int i = 0;
      while (emptyBounds && i < m_blocks.size())
      {
        Bounds blockBounds = m_blocks[i]->bounds();
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

          Bounds b1{bounds};
          Bounds b2{bounds};

          Nm splitPoint = (bounds[2*splitPlane] + bounds[2*splitPlane+1]) / 2.0;

          b1[2*splitPlane+1] = splitPoint;
          // Adjust splitPoint to voxel size
          auto region1 = equivalentRegion<T>(m_origin, m_spacing, b1);
          b1 = equivalentBounds<T>(m_origin, m_spacing, region1);          
          b2[2*splitPlane]   = b1[2*splitPlane+1];

          splitPlane = (splitPlane + 1)%3;

          remaining << SplitBounds(b1, splitPlane) << SplitBounds(b2, splitPlane);
        } else
        {
          blockBounds << bounds;
        }
      }
    }

    QList<typename T::Pointer> blockImages;

    for(auto bounds : blockBounds)
    {
      typename T::Pointer image = itkImage(bounds);
      bool empty = true;
      itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
      it.Begin();
      while (empty && !it.IsAtEnd())
      {
        empty = it.Get() != backgroundValue();
        ++it;
      }

      if (!empty) blockImages << itkImage(bounds);
    }

    m_blocks.clear();

    for(auto image : blockImages)
    {
      setBlock(image);
    }
  }
}
