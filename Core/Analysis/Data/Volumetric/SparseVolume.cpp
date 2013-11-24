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

// ITK
#include <itkImageRegionIterator.h>
#include <itkExtractImageFilter.h>
#include <itkMetaImageIO.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

// VTK
#include <vtkImplicitFunction.h>
#include <QDir>

namespace EspINA
{

  const int UNSET = 0;
  const int SET   = 1;

  //-----------------------------------------------------------------------------
  template<typename T>
  SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing) throw (Invalid_Image_Bounds_Exception)
  : m_spacing{spacing}
  , m_bounds{bounds}
  {
  //   if (!bounds.areValid())
  //     throw Invalid_Image_Bounds_Exception();

    this->setBackgroundValue(0);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  double SparseVolume<T>::memoryUsage() const
  {
    double numPixels = 0;

    for (unsigned int i = 0; i < m_blocks.size(); ++i)
      numPixels += m_blocks[i]->numberOfVoxels();

    return memory_size_in_MB(numPixels);
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
    m_origin = origin;
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setSpacing(const NmVector3& spacing)
  {
    m_spacing = spacing;
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::setBlock(typename T::Pointer image)
  {
    BlockUPtr block(new Block(image, false));
    m_blocks.push_back(std::move(block));

    Bounds bounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());

    updateBlocksBoundingBox(bounds);
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::addBlock(BlockMaskUPtr mask)
  {
    Bounds bounds = mask->bounds();

    BlockUPtr block(new Block(std::move(mask), false));
    m_blocks.push_back(std::move(block));

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
    int numVoxels    = imageRegion.GetNumberOfPixels();
    Q_ASSERT(numVoxels == mask->numberOfVoxels());

    if (m_blocks.size() == 0)
      return image;

    for (int i = m_blocks.size() -1; i >= 0; --i)
    {
      if (!intersect(bounds, m_blocks[i]->bounds()))
        continue;

      if (numVoxels == 0)
        break;

      Bounds intersectionBounds = intersection(bounds, m_blocks[i]->bounds());

      BinaryMask<unsigned char>::region_iterator mit(mask, intersectionBounds);
      itk::ImageRegionIterator<T> iit(image, equivalentRegion<T>(image, intersectionBounds));

      mit.goToBegin();
      iit = iit.Begin();
      switch(m_blocks[i]->type())
      {
        case BlockType::Set:
          {
            auto blockRegion = equivalentRegion<T>(m_blocks[i]->m_image, intersectionBounds);
            itk::ImageRegionIterator<T> bit(m_blocks[i]->m_image, blockRegion);
            bit = bit.Begin();
            while(!mit.isAtEnd())
            {
              if (!mit.isSet())
              {
                iit.Set(bit.Value());
                mit.Set();
                --numVoxels;
              }
              ++mit;
              ++bit;
              ++iit;
            }
          }
          break;
        case BlockType::Add:
          {
            BinaryMask<unsigned char>::region_iterator bit(m_blocks[i]->m_mask.get(), intersectionBounds);
            bit.goToBegin();

            while(!mit.isAtEnd())
            {
              if (!mit.isSet() && bit.isSet())
              {
                iit.Set(m_blocks[i]->m_mask->foregroundValue());
                mit.Set();
                --numVoxels;
              }
              ++mit;
              ++bit;
              ++iit;
            }
          }
          break;
        default:
          Q_ASSERT(false);
          break;
      }
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

    BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(intersectionBounds, m_spacing);
    mask->setForegroundValue(value);
    BinaryMask<unsigned char>::region_iterator it(mask, intersectionBounds);

    it.goToBegin();
    while (!it.isAtEnd())
    {
      BinaryMask<unsigned char>::IndexType index = it.getIndex();
      const double point[3]{ index.x * m_spacing[0], index.y * m_spacing[1], index.z * m_spacing[2]};
      if (const_cast<vtkImplicitFunction *>(brush)->FunctionValue(point[0], point[1], point[2]) <= 0)
        it.Set();
      ++it;
    }

    addBlock(BlockMaskUPtr(mask));
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void SparseVolume<T>::draw(const typename T::Pointer volume,
                             const Bounds&             bounds)
  {
    if (!intersect(m_bounds, bounds))
      return;

    Bounds intersectionBounds = intersection(m_bounds, bounds);

    typename T::Pointer block;

    if (intersectionBounds != m_bounds)
    {
      using ExtractorType = itk::ExtractImageFilter<T,T>;
      typename ExtractorType::Pointer extractor = ExtractorType::New();
      typename T::RegionType region = equivalentRegion<T>(volume, intersectionBounds);
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
      BinaryMask<unsigned char> *mask = new BinaryMask<unsigned char>(intersectionBounds, m_spacing);
      mask->setForegroundValue(value);
      BinaryMask<unsigned char>::region_iterator it(mask, intersectionBounds);
      it.goToBegin();
      it.Set();

      addBlock(BlockMaskUPtr(mask));
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

    m_spacing = NmVector3();

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
        }
      } else
      {
        for(int s=0; s < 3; ++s)
        {
          error |= m_spacing[s] != image->GetSpacing()[s];
        }
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


      Q_ASSERT(BlockType::Set == m_blocks[i]->type());

      auto volume = m_blocks[i]->m_image;
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
  void SparseVolume<T>::compact()
  {
    typename T::Pointer image = itkImage();

    m_blocks.clear();

    auto region = image->GetLargestPossibleRegion();

    setBlock(image);
  }

}
