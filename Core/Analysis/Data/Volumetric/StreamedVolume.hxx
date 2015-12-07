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

// ITK
#include <itkImageRegionIterator.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>

namespace ESPINA {

  template class VolumetricData<itk::Image<unsigned char, 3>>;

  /** \class StreamedVolume
   * \brief Class to stream a volume from disk.
   *
   */
  template<typename T>
  class StreamedVolume
  : public VolumetricData<T>
  {
    public:
      /** \brief StreamedVolume class constructor.
       *
       */
      explicit StreamedVolume();

      /** \brief StreamedVolume class constructor.
       * \param[in] fileName name of the image file used for streaming.
       *
       */
      explicit StreamedVolume(const QFileInfo& fileName);

      /** \brief StreamedVolume class virtual destructor.
       *
       */
      virtual ~StreamedVolume()
      {};

      /** \brief Sets the file name of the image file used for streaming.
       *
       */
      void setFileName(const QFileInfo& fileName);

      /** \brief Returns the file name of the image file used for streaming.
       *
       */
      QFileInfo fileName() const
      { return m_fileName; }

      virtual size_t memoryUsage() const override
      { return 0; }

      virtual VolumeBounds bounds() const override;

      virtual void setOrigin(const NmVector3& origin) override
      { m_origin = origin; }

      virtual void setSpacing(const NmVector3& spacing) override
      { m_spacing = spacing; }

      virtual const typename T::Pointer itkImage() const override;

      virtual const typename T::Pointer itkImage(const Bounds& bounds) const override;

      virtual void draw(vtkImplicitFunction*        brush,
                        const Bounds&               bounds,
                        const typename T::ValueType value)                   override
      {}

      virtual void draw(const typename T::Pointer volume)                    override
      {}

      virtual void draw(const typename T::Pointer volume,
                        const Bounds&             bounds)                    override
      {}

      virtual void draw(const typename T::IndexType &index,
                        const typename T::PixelType  value = SEG_VOXEL_VALUE) override
      {}

      virtual void draw(const Bounds               &bounds,
                        const typename T::PixelType value = SEG_VOXEL_VALUE) override
      {}

      virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                        const typename T::ValueType value = SEG_VOXEL_VALUE) override
      {}


      virtual void resize(const Bounds &bounds) override
      {}

      virtual bool isValid() const override
      { return QFileInfo(m_fileName).exists(); }

      virtual bool isEmpty() const override
      { return !isValid(); }

      virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const override
      { return Snapshot(); }

      virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) const override
      { return Snapshot(); }

      virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override
      {}

    protected:
      virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds) override
      { return false; }

    private:
      typedef itk::ImageRegionIterator<T> ImageIterator;

      virtual QList<Data::Type> updateDependencies() const override
      { return QList<Data::Type>(); }

    private:
      NmVector3 m_origin;
      NmVector3 m_spacing;

      Bounds  m_bounds;
      QString m_fileName;
      QString m_storageFileName;
  };

  template<typename T> using StreamReaderType  = itk::ImageFileReader<T>;
  template<typename T> using StreamExtractType = itk::ExtractImageFilter<T, T>;

  //-----------------------------------------------------------------------------
  template<typename T>
  StreamedVolume<T>::StreamedVolume()
  : m_origin {0, 0, 0}
  , m_spacing{1, 1, 1}
  {
    this->setBackgroundValue(0);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  StreamedVolume<T>::StreamedVolume(const QFileInfo &fileName)
  : m_fileName{fileName.absoluteFilePath()}
  {
    auto reader = StreamReaderType<T>::New();
    reader->ReleaseDataFlagOn();
    reader->SetFileName(m_fileName.toStdString());
    reader->UpdateOutputInformation();

    typename T::Pointer image = reader->GetOutput();

    for(int i = 0; i < 3; ++i)
    {
      m_origin[i]  = image->GetOrigin()[i];
      m_spacing[i] = image->GetSpacing()[i];
    }

    this->setBackgroundValue(0);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  VolumeBounds StreamedVolume<T>::bounds() const
  {
    if (!isValid())
    {
      auto what = QObject::tr("Uninitialized StreamedVolume.");
      auto details = QObject::tr("StreamedVolume::bounds() -> Uninitialized file.");

      throw Core::Utils::EspinaException(what, details);
    }

    auto reader = StreamReaderType<T>::New();
    reader->ReleaseDataFlagOn();
    reader->SetFileName(m_fileName.toStdString());
    reader->UpdateOutputInformation();

    typename T::Pointer image = reader->GetOutput();

    auto bounds = equivalentBounds<T>(m_origin, m_spacing, image->GetLargestPossibleRegion());

    return VolumeBounds(bounds, m_spacing, m_origin);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer StreamedVolume<T>::itkImage() const
  {
    if (!isValid())
    {
      auto what = QObject::tr("Uninitialized StreamedVolume.");
      auto details = QObject::tr("StreamedVolume::itkImage() -> Uninitialized file.");

      throw Core::Utils::EspinaException(what, details);
    }

    auto reader = StreamReaderType<T>::New();
    reader->ReleaseDataFlagOn();
    reader->SetFileName(m_fileName.toStdString());
    reader->Update();

    typename T::Pointer image = reader->GetOutput();
    image->DisconnectPipeline();

    image->SetSpacing(ItkSpacing<T>(m_spacing));

    return image;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer StreamedVolume<T>::itkImage(const Bounds& bounds) const
  {
    if (!isValid())
    {
      auto what = QObject::tr("Uninitialized StreamedVolume.");
      auto details = QObject::tr("StreamedVolume::itkImage(bounds) -> Uninitialized file.");

      throw Core::Utils::EspinaException(what, details);
    }

    auto reader = StreamReaderType<T>::New();
    reader->ReleaseDataFlagOn();
    reader->SetFileName(m_fileName.toStdString());
    reader->UpdateOutputInformation();

    auto requestedRegion = equivalentRegion<T>(m_origin, m_spacing, bounds);

    auto extractor = StreamExtractType<T>::New();
    extractor->SetExtractionRegion(requestedRegion);
    extractor->SetInput(reader->GetOutput());
    extractor->Update();

    typename T::Pointer image = extractor->GetOutput();
    image->DisconnectPipeline();

    image->SetSpacing(ItkSpacing<T>(m_spacing));

    return image;
  }
}

#endif // ESPINA_STREAMED_VOLUME_H
