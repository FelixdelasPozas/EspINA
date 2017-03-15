/*
 * Copyright (C) 2017, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#ifndef FILTERS_SLICEINTERPOLATIONFILTER_H_
#define FILTERS_SLICEINTERPOLATIONFILTER_H_

// ESPINA
#include "Filters/EspinaFilters_Export.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Filter.h>

// ITK
#include <itkShapeLabelObject.h>
#include <itkSmartPointer.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT SliceInterpolationFilter
  : public Filter
  {
    public:
      /** \brief SliceInterpolationFilter class constructor.
       * \param[in] inputs list of input smart pointers.
       * \param[in] type SliceInterpolationFilter type.
       * \param[in] scheduler scheduler smart pointer.
       *
       */
      SliceInterpolationFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler);

      /** \brief SliceInterpolationFilter class virtual destructor.
       *
       */
      virtual ~SliceInterpolationFilter()
      {}

      virtual void restoreState(const State& state)
      {}

      virtual State state() const
      { return State(); }

    protected:
      virtual Snapshot saveFilterSnapshot() const
      { return Snapshot(); }

      virtual bool needUpdate() const;

      virtual bool ignoreStorageContent() const
      { return false; }

      virtual void execute();

    private:
      using SLO = itk::SmartPointer<itk::ShapeLabelObject<itkVolumeType::SizeValueType,itkVolumeType::ImageDimension>>;

    private:
      class ContourInfo {
        public:
          ContourInfo()
          : m_max{0}
          , m_min{0}
          , m_average{0}
          {};

          ContourInfo(itkVolumeType::PixelType max, itkVolumeType::PixelType min, double average);
          const ContourInfo operator&&(const ContourInfo & other);
          const itkVolumeType::PixelType max() const;
          const itkVolumeType::PixelType min() const;
          const double average() const;
          const bool inRange(const unsigned char value) const;

          void print(std::ostream & os)
          {
            os << "[ min " << static_cast<int>(min()) << " max: " << static_cast<int>(max()) << " med: " << average() << " threshold " << (average() - static_cast<int>(min())) << "\n";
          }


        private:
          itkVolumeType::PixelType m_max;
          itkVolumeType::PixelType m_min;
          double m_average;

      };

    private:
      ContourInfo getContourInfo(itkVolumeType::Pointer image, Output::ReadLockData<DefaultVolumetricData> stackVolume, SLO slObject, Axis direction);
      bool belongToContour(itkVolumeType::IndexType index, SLO slObject, Axis direction);
      void printRegion(itkVolumeType::RegionType region);
      void printImageInZ(itkVolumeType::Pointer image, itkVolumeType::OffsetValueType offsetInZ = 0);

    private:
      QString m_errorMessage;
  };


  using SliceInterpolationFilterPtr = SliceInterpolationFilter *;
  using SliceInterpolationFilterSPtr = std::shared_ptr<SliceInterpolationFilter>;

} /* namespace ESPINA */

#endif /* FILTERS_SLICEINTERPOLATIONFILTER_H_ */
