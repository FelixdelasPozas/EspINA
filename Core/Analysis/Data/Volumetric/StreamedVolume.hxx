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

#ifndef ESPINA_STREAMED_VOLUME_H
#define ESPINA_STREAMED_VOLUME_H

// ESPINA
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/EspinaException.h>

// VTK
#include <vtkMath.h>

// ITK
#include <itkImageRegionIterator.h>

// Qt
#include <QMutex>

namespace ESPINA
{
  namespace Core
  {
    /** \class StreamedVolume
     * \brief Wrapper class around itk::Image for read-only large data files.
     *
     */
    template<typename T>
    class StreamedVolume
    : public VolumetricData<T>
    {
      public:
        /** \brief StreamedVolume class constructor.
         * \param[in] fileName name of the image file used for streaming.
         *
         */
        explicit StreamedVolume(const QFileInfo &fileName);

        /** \brief StreamedVolume class virtual destructor.
         *
         */
        virtual ~StreamedVolume()
        {};

        /** \brief Returns the file name of the image file used for streaming.
         *
         */
        QFileInfo fileName() const
        { return m_fileName; }

        virtual size_t memoryUsage() const override
        { return 0; }

        /** \brief Returns the pixel value vector size.
         *
         * NOTE: for itk::VectorImage the PixelType is just VariableLengthVector<type> and
         *       somehow the vector size is not in the T template parameter.
         *
         */
        unsigned int vectorLength() const
        { return m_vectorLength; }

        virtual VolumeBounds bounds() const override;

        virtual void setOrigin(const NmVector3& origin) override;

        virtual void setSpacing(const NmVector3& spacing) override;

        virtual const typename T::Pointer itkImage() const override;

        virtual const typename T::Pointer itkImage(const Bounds& bounds) const override;

        virtual void draw(vtkImplicitFunction*        brush,
                          const Bounds&               bounds,
                          const typename T::ValueType value) override;

        virtual void draw(const typename T::Pointer volume) override;

        virtual void draw(const typename T::Pointer volume,
                          const Bounds&             bounds) override;

        virtual void draw(const typename T::IndexType &index,
                          const typename T::PixelType  value = SEG_VOXEL_VALUE) override;

        virtual void draw(const Bounds               &bounds,
                          const typename T::PixelType value = SEG_VOXEL_VALUE) override;

        virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                          const typename T::ValueType value = SEG_VOXEL_VALUE) override;

        virtual void resize(const Bounds &bounds) override;

        virtual const typename T::RegionType itkRegion() const override;

        virtual const typename T::SpacingType itkSpacing() const override;

        virtual const typename T::PointType itkOriginalOrigin() const override;

        virtual bool isValid() const override
        { return QFileInfo(m_fileName).exists() && (m_vectorLength != 0); }

        virtual bool isEmpty() const override
        { return !isValid(); }

        virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override
        { return Snapshot(); }

        virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) override
        { return Snapshot(); }

        virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override
        {}

        /** \brief Returns the image corresponding to the given region.
         * \param[in] region itk region type.
         *
         * NOTE: this method is made available specially to operate with images with dimensions != 3.
         *
         */
        virtual const typename T::Pointer read(const typename T::RegionType &region) const;

        /** \brief Writes an image.
         * \param[in] image itk image smart pointer.
         *
         * NOTE: this method is made available specially to operate with images with dimensions != 3.
         *
         */
        virtual void write(const typename T::Pointer &image);

        /** \brief Fills the volume with the given value.
         * \param[in] value Pixel value.
         *
         */
        virtual void fill(const typename T::PixelType &value);

      protected:
        virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
        { return false; }

      private:
        virtual QList<Data::Type> updateDependencies() const override
        { return QList<Data::Type>(); }

      protected:
        StreamedVolume()
        : m_vectorLength{0}
        {};

        typename T::PointType   m_origin;       /** origin of the image on disk file. Should be {0,0,0} for images created with EspINA. */
        typename T::SpacingType m_spacing;      /** spacing of the image.                                                               */
        typename T::RegionType  m_region;       /** region index and size. Index can be different from {0,0,0} in EspINA files.         */
        unsigned int            m_vectorLength; /** length (or number of components per pixel) of the pixel value vector.               */
        QFileInfo               m_fileName;     /** file name of the file on disk.                                                      */
        mutable QReadWriteLock  m_lock;         /** lock for read/write ordered access.                                                 */
    };

    //-----------------------------------------------------------------------------
    template<typename T>
    StreamedVolume<T>::StreamedVolume(const QFileInfo &fileName)
    : m_fileName{fileName}
    {
      if(!fileName.exists())
      {
        auto message = QObject::tr("Invalid file name, doesn't exist: %1.").arg(fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::constructor() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      const auto shortName = getShortFileName(fileName.absoluteFilePath());

      auto reader = itk::ImageFileReader<T>::New();
      reader->ReleaseDataFlagOn();
      reader->SetFileName(shortName);
      reader->UseStreamingOff();
      reader->SetNumberOfThreads(1);
      reader->UpdateOutputInformation();

      auto image = reader->GetOutput();

      m_vectorLength = image->GetNumberOfComponentsPerPixel();
      if(m_vectorLength == 0)
      {
        auto message = QObject::tr("Invalid pixel value vector size.");
        auto details = QObject::tr("StreamedVolume::constructor() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
      {
        // fix ITK rounding error when reading doubles from file(i.e 1.1 will be read as 1.100000024).
        // and makes comparing spacings wrong, as internally operator== for vector does strict memcmp
        // comparing, like it should.
        m_origin.SetElement(i, QString::number(image->GetOrigin().GetElement(i)).toDouble());
        m_spacing.SetElement(i, QString::number(image->GetSpacing().GetElement(i)).toDouble());
      }
      m_region  = image->GetLargestPossibleRegion();

      // region must be updated because all regions in EspINA have an implicit origin of {0,0,0}
      for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
      {
        m_region.SetIndex(i, vtkMath::Round(m_origin.GetElement(i)/m_spacing.GetElement(i)));
      }

      typename T::PixelType pixelType;
      this->setBackgroundValue(itk::NumericTraits<typename T::PixelType>::ZeroValue(pixelType));
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    inline void StreamedVolume<T>::resize(const Bounds &bounds)
    {
      auto message = QObject::tr("Attempt to resize an static volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume::resize() -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::setOrigin(const NmVector3& origin)
    {
      QWriteLocker lock(&this->m_lock);

      m_origin = ItkPoint<T>(origin);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::setSpacing(const NmVector3& spacing)
    {
      QWriteLocker lock(&this->m_lock);

      m_spacing = ItkSpacing<T>(spacing);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    VolumeBounds StreamedVolume<T>::bounds() const
    {
      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::bounds() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::bounds() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      QReadLocker lock(&this->m_lock);

      NmVector3 origin{0, 0, 0}, spacing{m_spacing[0], m_spacing[1], m_spacing[2]};
      auto bounds  = equivalentBounds<T>(origin, spacing, m_region);

      return VolumeBounds(bounds, spacing, origin);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    const typename T::Pointer StreamedVolume<T>::itkImage() const
    {
      auto message = QObject::tr("Attemp to complete load an StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume::itkImage() -> ") + message;

      throw Core::Utils::EspinaException(message, details);

      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkImage() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      const auto shortName = getShortFileName(m_fileName.absoluteFilePath());

      QReadLocker lock(&this->m_lock);

      auto reader = itk::ImageFileReader<T>::New();
      reader->ReleaseDataFlagOn();
      reader->SetFileName(shortName);
      reader->SetUseStreaming(false);
      reader->SetNumberOfThreads(1);
      reader->Update();

      typename T::Pointer image = reader->GetOutput();
      image->DisconnectPipeline();
      image->SetNumberOfComponentsPerPixel(m_vectorLength);
      image->SetSpacing(m_spacing);

      auto origin = image->GetOrigin();
      for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
      {
        origin.SetElement(i,0);
      }

      image->SetOrigin(origin);
      image->SetRegions(m_region);

      return image;
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    const typename T::Pointer StreamedVolume<T>::itkImage(const Bounds& bounds) const
    {
      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkImage(bounds) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      const auto imageBounds = this->bounds();
      const auto eqRegion = equivalentRegion<T>(imageBounds.origin(), imageBounds.spacing(), bounds);

      return read(eqRegion);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(vtkImplicitFunction*        brush,
                                 const Bounds&               bounds,
                                 const typename T::ValueType value)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absolutePath());
      auto details = QObject::tr("StreamedVolume<>::draw(brush, bounds, value) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(const typename T::Pointer volume)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::draw(volume) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(const typename T::Pointer volume,
                                 const Bounds&             bounds)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::draw(volume, bounds) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(const typename T::IndexType &index,
                                 const typename T::PixelType  value)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::draw(index, value) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(const Bounds               &bounds,
                                 const typename T::PixelType value)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::draw(bounds, value) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                                 const typename T::ValueType value)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::draw(mask, value) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    const typename T::Pointer StreamedVolume<T>::read(const typename T::RegionType &region) const
    {
      QReadLocker lock(&this->m_lock);

      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkImage(bounds) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      if(!m_region.IsInside(region))
      {
        auto message = QObject::tr("Requested region is totally/partially outside the image region. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::read(region) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      // need to correct the region with the equivalent one on disk, just a displacement of origin length.
      typename T::RegionType requestedRegion = region;
      for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
      {
        requestedRegion.SetIndex(i, requestedRegion.GetIndex(i)-m_region.GetIndex(i));
      }

      const auto shortName = getShortFileName(m_fileName.absoluteFilePath());

      auto reader = itk::ImageFileReader<T>::New();
      reader->ReleaseDataFlagOn();
      reader->SetFileName(shortName);
      reader->SetNumberOfThreads(1);
      reader->UpdateOutputInformation();

      auto extractor = itk::ExtractImageFilter<T,T>::New();
      extractor->SetInput(reader->GetOutput());
      extractor->SetExtractionRegion(requestedRegion);
      extractor->Update();

      typename T::Pointer image = extractor->GetOutput();
      image->DisconnectPipeline();
      image->SetNumberOfComponentsPerPixel(m_vectorLength);
      image->SetSpacing(m_spacing);

      auto origin = image->GetOrigin();
      for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
      {
        origin.SetElement(i,0);
      }

      image->SetOrigin(origin);
      image->SetRegions(region);
      image->UpdateOutputInformation();

      return image;
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    inline void StreamedVolume<T>::write(const typename T::Pointer &image)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::write(image) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    inline const typename T::RegionType StreamedVolume<T>::itkRegion() const
    {
      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkRegion() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      QReadLocker lock(&this->m_lock);

      return m_region;
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    inline const typename T::SpacingType StreamedVolume<T>::itkSpacing() const
    {
      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkSpacing() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      QReadLocker lock(&this->m_lock);

      return m_spacing;
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    inline const typename T::PointType StreamedVolume<T>::itkOriginalOrigin() const
    {
      if (!isValid())
      {
        auto message = QObject::tr("Uninitialized StreamedVolume. File: %1").arg(m_fileName.absoluteFilePath());
        auto details = QObject::tr("StreamedVolume::itkOrigin() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      QReadLocker lock(&this->m_lock);

      return m_origin;
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void StreamedVolume<T>::fill(const typename T::PixelType &value)
    {
      auto message = QObject::tr("Attempt to modify a read-only volume. File: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("StreamedVolume<>::fill(&value) -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }

  } // namespace Core

  //-----------------------------------------------------------------------------
  template<typename T>
  QStringList filenames(Core::StreamedVolume<T> *image)
  {
    QStringList names;
    auto header = image->fileName().absoluteFilePath();
    auto raw    = header.left(header.lastIndexOf('.')) + QObject::tr(".raw");

    names << header << raw;

    return names;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void remove(Core::StreamedVolume<T> *image)
  {
    for(auto filename: filenames(image))
    {
      if (!QFile::remove(filename))
      {
        qWarning() << QObject::tr("Couldn't remove file '%1'").arg(filename);
      }
    }
  }

} // namespace ESPINA

#endif // ESPINA_STREAMED_VOLUME_H
