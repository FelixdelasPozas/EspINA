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
  // DEPRECATED
  template<typename T>
  class EspinaCore_EXPORT RawVolume
  : public VolumetricData<T>
  {
  public:
    /** \brief RawVolume class constructor
     * \param[in] bounds bounds of the empty volume.
     * \param[in] spacing spacing of the volume.
     * \param[in] origin origin of the volume.
     *
     */
    explicit RawVolume(const Bounds& bounds = Bounds(), const NmVector3& spacing = {1, 1, 1}, const NmVector3& origin = NmVector3());

    /** \brief RawVolume class constructor
     * \param[in] volume initial content of the raw volume.
     * \param[in] bounds bounds of the empty volume.
     * \param[in] spacing spacing of the volume.
     * \param[in] origin origin of the volume.
     *
     */
    explicit RawVolume(const typename T::Pointer volume, const Bounds& bounds, const NmVector3& spacing = {1, 1, 1}, const NmVector3& origin = NmVector3());

    explicit RawVolume(const typename T::Pointer volume);

    /** \brief RawVolume class destructor.
     *
     */
    virtual ~RawVolume()
    {}

    virtual size_t memoryUsage() const;

    virtual Bounds bounds() const;

    /** \brief Sets the origin of the volume.
     *
     */
    virtual void setOrigin(const NmVector3& origin);

    /** \brief Returns the origin of the volume.
     *
     */
    virtual NmVector3 origin() const;

    virtual void setSpacing(const NmVector3& spacing);

    virtual NmVector3 spacing() const
    { return m_spacing; }

    /** \brief Returns the equivalent itk image of the volume.
     *
     */
    virtual const typename T::Pointer itkImage() const;

    /** \brief Returns the equivalent itk image of a region of the volume.
     * \param[in] bounds equivalent bounds of the returning image.
     *
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) override;

    virtual void draw(const typename T::Pointer volume)                    override;

    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds)                    override;

    virtual void draw(const typename T::IndexType &index,
                      const typename T::ValueType  value = SEG_VOXEL_VALUE) override;

    virtual void draw(const Bounds               &bounds,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) override;

    virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) override;


    /** \brief Resize the volume bounds. The given bounds must containt the original.
     * \param[in] bounds bounds object.
     *
     */
    virtual void resize(const Bounds &bounds);

    virtual bool isValid() const;

    virtual bool isEmpty() const;

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const              override;

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override;

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)            override;

  protected:
    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override;

  private:
    QString editedRegionSnapshotId(const QString &outputId, const int regionId) const
    { return QString("%1_%2_EditedRegion_%3.mhd").arg(outputId).arg(this->type()).arg(regionId);}

  private:
    /** \brief Helper method to assist fetching data from disk.
     *
     */
    QString singleBlockPath(const QString &id) const
    { return QString("%1_%2.mhd").arg(id).arg(this->type()); }

    /** \brief Helper method to assist fetching data from disk.
     *
     */
    QString oldSingleBlockPath(const QString &id) const
    { return QString("%1_%2.mhd").arg(this->type()).arg(id); }

    virtual QList<Data::Type> updateDependencies() const override
    { return QList<Data::Type>(); }

  protected:
    NmVector3    m_origin;
    NmVector3    m_spacing;
    VolumeBounds m_bounds;

    typename T::Pointer m_image;
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  , m_origin {origin}
  , m_spacing{spacing}
  {
    m_bounds = VolumeBounds(bounds, m_spacing, m_origin);

    this->setBackgroundValue(SEG_BG_VALUE);

    m_image = create_itkImage<T>(bounds, SEG_BG_VALUE, spacing, origin);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const typename T::Pointer volume, const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  , m_origin {origin}
  , m_spacing{spacing}
  {
    m_image  = volume;
    m_bounds = volumeBounds<T>(m_image, m_image->GetLargestPossibleRegion());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const typename T::Pointer volume)
  : VolumetricData<T>()
  {
    m_image   = volume;
    m_spacing = ToNmVector3<T>(m_image->GetSpacing());
    m_origin  = ToNmVector3<T>(m_image->GetOrigin());
    m_bounds  = volumeBounds<T>(m_image, m_image->GetLargestPossibleRegion());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  size_t RawVolume<T>::memoryUsage() const
  {
    return m_image->GetBufferedRegion().GetNumberOfPixels()*sizeof(typename T::ValueType);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Bounds RawVolume<T>::bounds() const
  {
    return m_bounds.bounds();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::setOrigin(const NmVector3& origin)
  {
    //NmVector3 shift = m_origin - origin;
    m_image->SetOrigin(ItkPoint<T>(origin));
    m_image->Update();

    m_origin = origin;
    m_bounds = volumeBounds<T>(m_image, m_image->GetLargestPossibleRegion());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  NmVector3 RawVolume<T>::origin() const
  {
    return m_origin;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::setSpacing(const NmVector3& spacing)
  {
    if (m_spacing != spacing)
    {
      m_image->SetSpacing(ItkSpacing<T>(spacing));
      m_image->Update();

      m_spacing = spacing;
      m_bounds  = volumeBounds<T>(m_image, m_image->GetLargestPossibleRegion());
    }
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer RawVolume<T>::itkImage() const
  {
    return m_image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer RawVolume<T>::itkImage(const Bounds& bounds) const
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

    auto region = equivalentRegion<T>(m_image, expectedBounds.bounds());

    if(!m_image->GetLargestPossibleRegion().IsInside(region))
    {
      qDebug() << "avoiding itk::exception";
      region.Print(std::cout);
      m_image->GetLargestPossibleRegion().Print(std::cout);
      std::cout << std::flush;

      region.Crop(m_image->GetLargestPossibleRegion());
    }

    auto extractor = itk::ExtractImageFilter<T,T>::New();
    extractor->SetInput(m_image);
    extractor->SetExtractionRegion(region);
    extractor->Update();

    return extractor->GetOutput();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const vtkImplicitFunction  *brush,
                          const Bounds               &bounds,
                          const typename T::ValueType value)
  {
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const typename T::Pointer volume)
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const typename T::Pointer volume,
                          const Bounds&             bounds)
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void RawVolume<T>::draw(const typename T::IndexType &index,
                          const typename T::ValueType value)
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void RawVolume<T>::draw(const Bounds               &bounds,
                             const typename T::ValueType value)
  {
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::resize(const Bounds &bounds)
  {
    Q_ASSERT(false);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool RawVolume<T>::isValid() const
  {
    return m_bounds.areValid() && !this->needFetch();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool RawVolume<T>::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
  {
//     using VolumeReader = itk::ImageFileReader<itkVolumeType>;
//
    bool dataFetched = false;
    bool error       = false;
//
//     int i = 0;
//     QFileInfo blockFile;
//
//     auto output  = this->m_output;
//
//     if (nullptr == output)
//     {
//       qWarning() << "Raw Volume Fetch Data without output";
//     }
//
//     if (!storage || path.isEmpty() || id.isEmpty()) return false;
//
//     for (auto filename : {multiBlockPath    (id, i),
//                           singleBlockPath   (id)   ,
//                           oldMultiBlockPath (id, i), // This shouldn't exist
//                           oldSingleBlockPath(id)})
//     {
//       blockFile = QFileInfo(storage->absoluteFilePath(path + filename));
//       if (blockFile.exists()) break;
//     }
//
//     if (output)
//     {
//       m_spacing = output->spacing();
//     }
//
//     while (blockFile.exists())
//     {
//       VolumeReader::Pointer reader = VolumeReader::New();
//       reader->SetFileName(blockFile.absoluteFilePath().toUtf8().data());
//       reader->Update();
//
//       auto image = reader->GetOutput();
//
//       if (m_spacing == NmVector3() || output == nullptr)
//       {
//         for(int s=0; s < 3; ++s)
//         {
//           m_spacing[s] = image->GetSpacing()[s];
//         }
//
//         if (output)
//         {
//           output->setSpacing(m_spacing);
//         }
//       } else
//       {
//         image->SetSpacing(ItkSpacing<T>(m_spacing));
//       }
//
//       setBlock(image, true);
//
//       ++i;
//       blockFile = storage->absoluteFilePath(path + multiBlockPath(id, i));
//       dataFetched = true;
//     }
//
//     m_bounds = m_blocksBounds;
//
    return dataFetched && !error;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot RawVolume<T>::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
  {
    Snapshot snapshot;
/*
    auto compactedBounds = compactedBlocks();

    for(int i = 0; i < compactedBounds.size(); ++i)
    {
      auto bounds = compactedBounds[i].bounds();

      QString filename = path;

      if (compactedBounds.size() == 1)
      {
        filename = singleBlockPath(id);
      }
      else
      {
        filename = multiBlockPath(id, i);
      }

      snapshot << createSnapshot<T>(itkImage(bounds), storage, path, filename);

    } */

    return snapshot;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot RawVolume<T>::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const
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
  void RawVolume<T>::restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    auto restoredEditedRegions = this->editedRegions();

    for (int regionId = 0; regionId < restoredEditedRegions.size(); ++regionId)
    {
      QFileInfo filename(storage->absoluteFilePath(path + "/" + editedRegionSnapshotId(id, regionId)));

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
  bool RawVolume<T>::isEmpty() const
  {
    if(!isValid()) return true;

    itk::ImageRegionIterator<T> it(m_image, m_image->GetLargestPossibleRegion());
    it.GoToBegin();
    while (!it.IsAtEnd())
    {
      if(it.Get() != this->backgroundValue())
        return false;

      ++it;
    }

    return true;
  }

  using RawVolumePtr  = RawVolume<itkVolumeType> *;
  using RawVolumeSPtr = std::shared_ptr<RawVolume<itkVolumeType>>;
}

#endif // ESPINA_SPARSE_VOLUME_H
