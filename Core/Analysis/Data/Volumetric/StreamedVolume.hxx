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
#include "Core/Analysis/Data/VolumetricData.hxx"
#include "Core/Analysis/Data/VolumetricDataUtils.hxx"
#include "Core/Utils/BinaryMask.hxx"
#include "Core/Utils/Bounds.h"

// ITK
#include <itkImageRegionIterator.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>

namespace ESPINA {

  template class VolumetricData<itk::Image<unsigned char, 3>>;

  template<typename T>
  class StreamedVolume
  : public VolumetricData<T>
  {
  public:
    struct File_Not_Found_Exception{};

  public:
    /* \brief StreamedVolume class constructor.
     *
     */
    explicit StreamedVolume();

    /* \brief StreamedVolume class constructor.
     * \param[in] fileName, name of the image file used for streaming.
     *
     */
    explicit StreamedVolume(const QFileInfo& fileName);

    /* \brief StreamedVolume class virtual destructor.
     *
     */
    virtual ~StreamedVolume()
    {};

    /* \brief Sets the file name of the image file used for streaming.
     *
     */
    void setFileName(const QFileInfo& fileName);

    /* \brief Returns the file name of the image file used for streaming.
     *
     */
    QFileInfo fileName() const
    { return m_fileName; }

    /* \brief Implements Data::memoryUsage().
     *
     */
    virtual size_t memoryUsage() const
    { return 0; }

    /* \brief Overrides Data::bounds() const.
     *
     */
    virtual Bounds bounds() const;

    /* \brief Implements VolumetricData<T>::setOrigin().
     *
     */
    virtual void setOrigin(const NmVector3& origin)
    { m_origin = origin; }

    /* \brief Implements VolumetricData<T>::origin().
     *
     */
    virtual NmVector3 origin() const
    { return m_origin; }

    /* \brief Implements Data::setSpacing().
     *
     */
    virtual void setSpacing(const NmVector3& spacing)
    { m_spacing = spacing; }

    /* \brief Implements Data::spacing() const.
     *
     */
    virtual NmVector3 spacing() const;

    /* \brief Implements VolumetricData<T>::itkImage() const.
     *
     */
    virtual const typename T::Pointer itkImage() const;

    /* \brief Implements VolumetricData<T>::itkImage(Bounds) const.
     *
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    /* \brief Implements VolumetricData<T>::draw(vtkImplicitFunction, Bounds, T::ValueType).
     *
     */
    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value)
    {}

    /* \brief Implements VolumetricData<T>::draw(T::Pointer).
     *
     */
    virtual void draw(const typename T::Pointer volume)
    {}

    /* \brief Implements VolumetricData<T>::draw(T::Pointer, Bounds)
     *
     */
    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds)
    {}

    /* \brief Implements VolumetricData<T>::draw(T::IndexType, T::ValueType)
     *
     */
    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE)
    {}

    /* \brief Implements VolumetricData<T>::resize().
     *
     */
    virtual void resize(const Bounds &bounds)
    {}

    /* \brief Implements Data::undo().
     *
     */
    virtual void undo()
    {}

    /* \brief Implements Data::isValid().
     *
     */
    virtual bool isValid() const
    { return QFileInfo(m_fileName).exists(); }

    /* \brief Implements Data::isEmpty().
     *
     */
    virtual bool isEmpty() const
    { return !isValid(); }

    /* \brief Implements Data::fetchData().
     *
     */
    virtual bool fetchData(TemporalStorageSPtr storage, const QString& prefix)
    { return false; }

    /* \brief Implements Data::snapshot().
     *
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const
    { return Snapshot(); }

    /* \brief Implements Data::editedRegionsSnapshot().
     *
     */
    virtual Snapshot editedRegionsSnapshot() const
    { return Snapshot(); }

  protected:
    typedef itk::ImageRegionIterator<T> ImageIterator;

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
  Bounds StreamedVolume<T>::bounds() const
  {
    if (!isValid()) throw File_Not_Found_Exception();

    auto reader = StreamReaderType<T>::New();
    reader->ReleaseDataFlagOn();
    reader->SetFileName(m_fileName.toStdString());
    reader->UpdateOutputInformation();

    typename T::Pointer image = reader->GetOutput();

    return equivalentBounds<T>(m_origin, m_spacing, image->GetLargestPossibleRegion());
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  NmVector3 StreamedVolume<T>::spacing() const
  {
    if (!isValid()) throw File_Not_Found_Exception();

    return m_spacing;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::Pointer StreamedVolume<T>::itkImage() const
  {
    if (!isValid()) throw File_Not_Found_Exception();

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
    if (!isValid()) throw File_Not_Found_Exception();

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
