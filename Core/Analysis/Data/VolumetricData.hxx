/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_VOLUMETRIC_DATA_H
#define ESPINA_VOLUMETRIC_DATA_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Analysis/Data.h"
#include "Core/Analysis/DataProxy.h"
#include <Core/Analysis/Output.h>
#include <Core/Analysis/Data/Volumetric/VolumetricDataProxy.hxx>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

// C++
#include <memory>

class vtkImplicitFunction;

namespace ESPINA
{
  template<typename T>
  class EspinaCore_EXPORT VolumetricData
  : public Data
  {
  public:
    static const Data::Type TYPE;

  public:
    /** \brief VolumetricData class constructor.
     *
     */
    explicit VolumetricData()
    : m_bgValue(0)
    {}

    /** \brief VolumetricData class destructor.
     *
     */
    virtual ~VolumetricData()
    {}

    /** \brief Implements Data::bounds() const.
     *
     */
    virtual Bounds bounds() const = 0;

    /** \brief Implements Data::type() const.
     *
     */
    virtual Data::Type type() const
    { return TYPE; }

    /** \brief Implements Data::createProxy() const.
     *
     */
    virtual DataProxySPtr createProxy() const
    { return DataProxySPtr{new VolumetricDataProxy<T>()}; }

    /** \brief Set the origin of the image.
     * \param[in] origin, origin of this image.
     *
     */
    virtual void setOrigin(const NmVector3& origin) = 0;

    /** \brief Returns the origin of the image.
     *
     */
    virtual NmVector3 origin() const = 0;

    /** \brief Return a read only ItkImage equivalent to the whole volume representation.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const typename T::Pointer itkImage() const  = 0;

    /** \brief Return a read only ItkImage of volume representation contained in bounds.
     * \param[in] bounds, bounds of the resulting image.
     *
     * This may request extra memory to allocate the requested region.
     */
    virtual const typename T::Pointer itkImage(const Bounds& bounds) const = 0;

    /** \brief Set volume background value
     * \param[in] value, background value.
     *
     */
    virtual void setBackgroundValue(const typename T::ValueType value)
    {  m_bgValue = value; }

    /** \brief Return volume background value
     *
     */
    virtual typename T::ValueType backgroundValue() const
    {  return m_bgValue; }

    /** \brief Method to modify the volume using a implicit function.
     * \param[in] brush, vtkImplicitFunction raw pointer.
     * \param[in] bounds, bounds to constrain the modification.
     * \param[in] value, value of voxels that comply with the function.
     *
     *  Change every voxel value which satisfies the implicit function to
     *  the value given as parameter.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value) = 0;

    /** \brief Method to modify the volume using an itk image.
     * \param[in] volume, itk image smart pointer.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const typename T::Pointer volume) = 0;

    /** \brief Method to modify the volume using a region of an itk image.
     * \param[in] volume, itk image smart pointer.
     * \param[in] bounds, bounds to constrain the modification.
     *
     *  Draw methods are constrained to sparse volume bounds.
     */
    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds) = 0;

    /** \brief Set voxel at index to value.
     * \param[in] index, index to modify value.
     * \param[in] value, new value.
     *
     */
    virtual void draw(const typename T::IndexType index,
                      const typename T::ValueType value = SEG_VOXEL_VALUE) = 0;

    /** \brief Resize the volume to the given bounds.
     * \param[in] bounds, new bounds.
     *
     *  New voxels will be set to background value
     */
    virtual void resize(const Bounds &bounds) = 0;

  private:
    typename T::ValueType m_bgValue;

    friend class Output;
  };

  template<typename T>
  const Data::Type VolumetricData<T>::TYPE = "VolumetricData";

  template<class T> using VolumetricDataPtr  = VolumetricData<T> *;
  template<class T> using VolumetricDataSPtr = std::shared_ptr<VolumetricData<T>>;

  using DefaultVolumetricDataSPtr = std::shared_ptr<VolumetricData<itkVolumeType>>;

  /** \brief Obtains and returns the VolumetricData smart pointer in the specified Output.
   * \param[in] output, Output object smart pointer.
   *
   */
  DefaultVolumetricDataSPtr EspinaCore_EXPORT volumetricData(OutputSPtr output);

} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_DATA_H