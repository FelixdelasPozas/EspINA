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

#ifndef ESPINA_VOLUMETRIC_DATA_PROXY_H
#define ESPINA_VOLUMETRIC_DATA_PROXY_H

// ESPINA
#include "Core/Analysis/Data/VolumetricData.hxx"
#include <Core/Analysis/DataProxy.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class vtkImplicitFunction;
namespace ESPINA
{
  template <class T> class VolumetricData;

  template<typename T>
  class EspinaCore_EXPORT VolumetricDataProxy
  : public VolumetricData<T>
  , public DataProxy
  {
		public:
			/** brief VolumetricDataProxy class constructor.
			 *
			 */
			explicit VolumetricDataProxy()
			{}

			/** brief VolumetricDataProxy class virtual destructor.
			 *
			 */
			virtual ~VolumetricDataProxy()
			{}

			/** brief Implements DataProxy::set().
			 *
			 */
			virtual void set(DataSPtr data)
			{
				m_data = std::dynamic_pointer_cast<VolumetricData<T>>(data);
				m_data->setOutput(this->m_output);
			}

			/** brief Implements DataProxy::get() const
			 *
			 */
			virtual DataSPtr get() const
			{ return m_data; }

			/** brief Implements Data::memoryUsage() const.
			 *
			 */
			virtual size_t memoryUsage() const
			{ return m_data->memoryUsage(); }

			/** brief Overrides VolumetricData<T>::bounds() const.
			 *
			 */
			virtual Bounds bounds() const
			{ return m_data->bounds(); }

			/** brief Implements VolumetricData<T>::setOrigin().
			 *
			 */
			virtual void setOrigin(const NmVector3& origin)
			{ m_data->setOrigin(origin); }

			/** brief Implements VolumetricData<T>::origin() const.
			 *
			 */
			virtual NmVector3 origin() const
			{ return m_data->origin(); }

			/** brief Implements Data::setSpacing().
			 *
			 */
			virtual void setSpacing(const NmVector3& spacing)
			{ m_data->setSpacing(spacing); }

			/** brief Implements Data::spacing() const.
			 *
			 */
			virtual NmVector3 spacing() const
			{ return m_data->spacing(); }

			/** brief Implements VolumetricData<T>::itkImage() const.
			 *
			 */
			virtual const typename T::Pointer itkImage() const
			{ return m_data->itkImage(); }

			/** brief Implements VolumetricData<T>::itkImage(Bounds) const.
			 *
			 */
			virtual const typename T::Pointer itkImage(const Bounds& bounds) const
			{ return m_data->itkImage(bounds); }

			/** brief Implements VolumetricData<T>::setBackgroundValue().
			 *
			 */
			virtual void setBackgroundValue(const typename T::ValueType value)
			{  m_data->setBackgroundValue(value); }

			/** brief Implements VolumetricData<T>::backgroundValue().
			 *
			 */
			typename T::ValueType backgroundValue() const
			{  return m_data->backgroundValue(); }

			/** brief Implements VolumetricData<T>::draw(vtkImplicitFunction, Bounds, T::ValueType).
			 *
			 */
			virtual void draw(const vtkImplicitFunction* brush,
												const Bounds&      bounds,
												const typename T::ValueType value)
			{ m_data->draw(brush, bounds, value); }

			/** brief Implements VolumetricData<T>::draw(typename T::Pointer).
			 *
			 */
			virtual void draw(const typename T::Pointer volume)
			{ m_data->draw(volume); }

			/** brief Implements VolumetricData<T>::draw(typename T::Pointer, Bounds).
			 *
			 */
			virtual void draw(const typename T::Pointer volume,
												const Bounds&             bounds)
			{ m_data->draw(volume, bounds); }

			/** brief Implements VolumetricData<T>::draw(T::IndexType, T::PixelType).
			 *
			 */
			virtual void draw(const typename T::IndexType index,
												const typename T::PixelType value = SEG_VOXEL_VALUE)
			{ m_data->draw(index, value); }

			/** brief Implements VolumetricData<T>::resize().
			 *
			 */
			virtual void resize(const Bounds &bounds)
			{ m_data->resize(bounds); }

			/** \brief Implements Data::undo().
			 *
			 */
			virtual void undo()
			{ m_data->undo(); }

			/** brief Implements Data::lastModified().
			 *
			 */
			virtual TimeStamp lastModified()
			{ return m_data->lastModified(); }

			/** brief Implements Data::editedRegions() const.
			 *
			 */
			virtual BoundsList editedRegions() const
			{ return m_data->editedRegions(); }

			/** brief Implements Data::clearEditedRegions().
			 *
			 */
			virtual void clearEditedRegions()
			{ m_data->clearEditedRegions(); }

			/** brief Implements Data::isValid().
			 *
			 */
			virtual bool isValid() const
			{ return m_data->isValid(); }

			/** brief Implements Data::isEmpty().
			 *
			 */
			virtual bool isEmpty() const
			{ return m_data->isEmpty(); }

			/** brief Implements Data::fetchData().
			 *
			 */
			virtual bool fetchData(TemporalStorageSPtr storage, const QString& prefix)
			{ return m_data->fetchData(storage, prefix); }

			/** brief Implements Data::snapshot().
			 *
			 */
			virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const
			{ return m_data->snapshot(storage, prefix); }

			/** brief Implements Data::editedRegionsSnapshot().
			 *
			 */
			virtual Snapshot editedRegionsSnapshot() const
			{ return m_data->editedRegionsSnapshot(); }

		private:
			std::shared_ptr<VolumetricData<T>> m_data;

			friend class Output;
  };
} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_DATA_PROXY_H
