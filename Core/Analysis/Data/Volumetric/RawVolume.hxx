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
#include <Core/Utils/EspinaException.h>

// ITK
#include <itkImageRegionIterator.h>

// VTK
#include <vtkImplicitFunction.h>

namespace ESPINA
{
  /** \brief Volume representation intended to save memory and speed up
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

    /** \brief RawVolume class cosntructor.
     * \param[in] volume itk volume pointer.
     *
     */
    explicit RawVolume(const typename T::Pointer volume);

    /** \brief RawVolume class destructor.
     *
     */
    virtual ~RawVolume()
    {}

    /** \brief Returns the memory used by the volume in bytes.
     *
     */
    virtual size_t memoryUsage() const override;

    /** \brief Sets the origin of the volume.
     * \param[in] origin origin point in Nm.
     *
     */
    virtual void setOrigin(const NmVector3& origin) override;

    /** \brief Sets the space of the volume.
     * \param[in] spacing spacing in Nm.
     */
    virtual void setSpacing(const NmVector3& spacing) override;

    /** \brief Returns the equivalent itk image of the volume.
     *
     */
    virtual const typename T::Pointer itkImage() const override;

    /** \brief Returns the equivalent itk image of a region of the volume.
     * \param[in] bounds equivalent bounds of the returning image.
     *
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const override;

    virtual void draw(vtkImplicitFunction*        brush,
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
    virtual void resize(const Bounds &bounds) override;

    virtual const typename T::RegionType itkRegion() const override;

    virtual const typename T::SpacingType itkSpacing() const override;

    virtual const typename T::PointType itkOriginalOrigin() const override;

    virtual bool isValid() const override;

    virtual bool isEmpty() const override;

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) override;

    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id) override;

    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id) override;

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
    typename T::Pointer m_image;
  };

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  {
    this->m_bounds = VolumeBounds(bounds, spacing, origin);

    this->setBackgroundValue(SEG_BG_VALUE);

    m_image = create_itkImage<T>(bounds, SEG_BG_VALUE, spacing, origin);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const typename T::Pointer volume, const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  : VolumetricData<T>()
  {
    m_image  = volume;
    this->m_bounds = volumeBounds<T>(m_image);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  RawVolume<T>::RawVolume(const typename T::Pointer volume)
  : VolumetricData<T>()
  {
    m_image       = volume;
    this->m_bounds = volumeBounds<T>(m_image);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  size_t RawVolume<T>::memoryUsage() const
  {
    return m_image->GetBufferedRegion().GetNumberOfPixels()*sizeof(typename T::ValueType);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::setOrigin(const NmVector3& origin)
  {
    //NmVector3 shift = m_origin - origin;
    m_image->SetOrigin(ItkPoint<T>(origin));
    m_image->Update();

    this->m_bounds = volumeBounds<T>(m_image);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::setSpacing(const NmVector3& spacing)
  {
    auto prevSpacing = this->m_bounds.spacing();
    if (prevSpacing != spacing)
    {
      m_image->SetSpacing(ItkSpacing<T>(spacing));
      m_image->Update();

      this->m_bounds = volumeBounds<T>(m_image);
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
      auto what    = QObject::tr("Invalid bounds");
      auto details = QObject::tr("RawVolume::ItkImage(bounds) -> Invalid input bounds.");

      throw Core::Utils::EspinaException(what, details);
    }

    auto origin  = this->m_bounds.origin();
    auto spacing = this->m_bounds.spacing();
    VolumeBounds expectedBounds(bounds, spacing, origin);

    if (!contains(this->m_bounds, expectedBounds))
    {
      auto what    = QObject::tr("Invalid bounds");
      auto details = QObject::tr("RawVolume::ItkImage(bounds) -> Bounds not contained in volume bounds, requested: %1, have: %2").arg(bounds.toString()).arg(this->m_bounds.toString());

      throw Core::Utils::EspinaException(what, details);
    }

    auto region = equivalentRegion<T>(m_image, expectedBounds.bounds());

    return extract_image<T>(m_image, region);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(vtkImplicitFunction        *brush,
                          const Bounds               &bounds,
                          const typename T::ValueType value)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(vtkImplicit) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }


  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                             const typename T::ValueType value)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(BinaryMask) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const typename T::Pointer volume)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(T::Pointer) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::draw(const typename T::Pointer volume,
                          const Bounds&             bounds)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(T::Pointer, Bounds) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void RawVolume<T>::draw(const typename T::IndexType &index,
                          const typename T::ValueType value)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(T::IndexType, T::ValueType) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void RawVolume<T>::draw(const Bounds               &bounds,
                             const typename T::ValueType value)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::draw(Bounds, T::ValueType) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::resize(const Bounds &bounds)
  {
    auto message = QObject::tr("Modification or RawVolume not allowed.");
    auto details = QObject::tr("RawVolume<T>::resize() -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool RawVolume<T>::isValid() const
  {
    return this->m_bounds.areValid() && !this->needFetch();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool RawVolume<T>::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
  {
    bool dataFetched = false;
    bool error       = false;

    return dataFetched && !error;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot RawVolume<T>::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id)
  {
    return Snapshot();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  Snapshot RawVolume<T>::editedRegionsSnapshot(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
    return Snapshot();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void RawVolume<T>::restoreEditedRegions(TemporalStorageSPtr storage, const QString& path, const QString& id)
  {
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
      if(it.Get() != this->backgroundValue()) return false;

      ++it;
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::RegionType RawVolume<T>::itkRegion() const
  {
    return this->m_image->GetLargestPossibleRegion();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::SpacingType RawVolume<T>::itkSpacing() const
  {
    return this->m_image->GetSpacing();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  const typename T::PointType RawVolume<T>::itkOriginalOrigin() const
  {
    return this->m_image->GetOrigin();
  }

  using RawVolumePtr  = RawVolume<itkVolumeType> *;
  using RawVolumeSPtr = std::shared_ptr<RawVolume<itkVolumeType>>;
}

#endif // ESPINA_SPARSE_VOLUME_H
