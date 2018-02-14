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
#include <itkLabelMap.h>
#include <itkShapeLabelObject.h>
#include <itkSmartPointer.h>
// ITK
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkLabelMapToBinaryImageFilter.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT SliceInterpolationFilter: public Filter
  {
    private:
      using RegionType = itkVolumeType::RegionType;
      using SizeValueType = itkVolumeType::SizeValueType;
      using PixelCounterType = unsigned long;
      using Histogram = std::vector<PixelCounterType>;
      //using HistogramSptr = std::shared_ptr<Histogram>;
      using SLO = itk::ShapeLabelObject<SizeValueType,itkVolumeType::ImageDimension>;
      using SLOSptr = itk::SmartPointer<SLO>;
      using ShapeLabelMap = itk::LabelMap<SLO>;
      using ShapeLabelMapToBinaryImageFilter = itk::LabelMapToBinaryImageFilter<ShapeLabelMap, itkVolumeType>;
      using BinaryImageToShapeLabelMapFilter = itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>;
      using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;
      using BinaryErodeFilter = itk::BinaryErodeImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;
      using BinaryDilateFilter = itk::BinaryDilateImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;

      class ContourInfo
      {
        public:
          ContourInfo();
          ContourInfo(PixelCounterType inlandMode, PixelCounterType beachMode, PixelCounterType coastMode, PixelCounterType seaMode,
                      itkVolumeType::Pointer contourMask);
          PixelCounterType getInlandMode() const;
          PixelCounterType getBeachMode() const;
          PixelCounterType getCoastMode() const;
          PixelCounterType getSeaMode() const;
          itkVolumeType::Pointer getContourMask() const; /* Mask with the following values: inland = 255, beach = 2, coast = 1  and sea = 0 */
          void setContourMask(itkVolumeType::Pointer image);

          void print(std::ostream & os) const;

        private:
          PixelCounterType m_inland_mode;
          PixelCounterType m_beach_mode;
          PixelCounterType m_coast_mode;
          PixelCounterType m_sea_mode;

          itkVolumeType::Pointer m_contour_mask;
      };

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
      {
      }

      virtual void restoreState(const State& state)
      {
      }

      virtual State state() const
      {
        return State();
      }

    protected:
      virtual Snapshot saveFilterSnapshot() const
      {
        return Snapshot();
      }

      virtual bool needUpdate() const;

      virtual bool ignoreStorageContent() const
      {
        return false;
      }

      virtual void execute();

    private:
      ContourInfo getContourInfo(const itkVolumeType::Pointer stackImage, const itkVolumeType::Pointer sloImage, const Axis direction,
                                 const SizeValueType bufferSize) const;
      itkVolumeType::Pointer sloToImage(const SLOSptr slObject, RegionType region);
      RegionType calculateRoi(const RegionType& maxRegion, const RegionType& srcRegion, const RegionType& tarRegion, const Axis direction,
                              const int extraOffset = 0);

      void printRegion(const RegionType region) const;
      void printImageInZ(const itkVolumeType::Pointer image, const itkVolumeType::OffsetValueType offsetInZ = 0) const;

    private:
      QString m_errorMessage;
  };

  using SliceInterpolationFilterPtr = SliceInterpolationFilter *;
  using SliceInterpolationFilterSPtr = std::shared_ptr<SliceInterpolationFilter>;

} /* namespace ESPINA */

#endif /* FILTERS_SLICEINTERPOLATIONFILTER_H_ */
