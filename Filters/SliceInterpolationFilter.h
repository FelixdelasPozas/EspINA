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
#include <Core/Utils/Histogram.h>
#include <Extensions/SLIC/StackSLIC.h>

// ITK
#include <itkBinaryBallStructuringElement.h>
#include <itkLabelMap.h>
#include <itkShapeLabelObject.h>
#include <itkSmartPointer.h>

namespace ESPINA
{
  /** \class SliceInterpolationFilter
   * \brief Filter that fills the empty space between slices interpolating between them.
   *
   */
  class EspinaFilters_EXPORT SliceInterpolationFilter
  : public Filter
  {
    private:
      using RegionType    = itkVolumeType::RegionType;
      using SpacingType   = itkVolumeType::SpacingType;
      using OriginType    = itkVolumeType::PointType;
      using SizeValueType = itkVolumeType::SizeValueType;
      using SLO           = itk::ShapeLabelObject<SizeValueType,itkVolumeType::ImageDimension>;
      using ShapeLabelMap = itk::LabelMap<SLO>;
      using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;

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

      virtual const QStringList errors() const;

      virtual void restoreState(const State& state)
      {}

      virtual State state() const
      {return State();}

      /** \brief Sets the use of SLIC clustering by the filter.
       * \parma[in] value True to use SLIC false to use thresholding.
       *
       */
      void setUseSLIC(const bool value)
      { m_useSLIC = value; }

      /** \brief Returns true if the filter uses SLIC and false if it uses thresholding.
       *
       */
      const bool useSLIC() const
      { return m_useSLIC; }

      /** \brief Sets the threshold value of the filter.
       * \param[in] value Threshold value in [0.1-0.9].
       *
       */
      void setThreshold(const double value)
      { m_threshold = std::min(0.9, std::max(0.1, value)); }

      /** \brief Returns the threshold value of the filter.
       *
       */
      const double thresholdValue() const
      { return m_threshold; }

      /** \brief Sets the SLIC extension.
       * \param[in] extension SLIC extension pointer.
       *
       */
      void setSLICExtension(Extensions::StackSLIC *extension)
      { m_slic = extension; }

    protected:
      virtual Snapshot saveFilterSnapshot() const
      { return Snapshot(); }

      virtual bool needUpdate() const;

      virtual bool ignoreStorageContent() const
      { return false; }

      virtual void execute();

    private:
      /** \brief Helper method that processes a single segmentation slice. Returns the processed slice.
       * \param[in] stackSlice Slice of the stack.
       * \param[in] segSlice   Slice of the segmentation.
       * \param[in] direction  Operation direction.
       * \param[in] histogram  Histogram of the values of the given input.
       */
      itkVolumeType::Pointer processSlice(const itkVolumeType::Pointer stackImage,
                                          const itkVolumeType::Pointer segImage,
                                          const Axis direction,
                                          const Core::Utils::Histogram &histogram) const;

      /** \brief Helper method that converts a ShapeLabelObject into an itk image with the given spacing and origin.
       * \param[in] sloObject ShapeLabelObject pointer.
       * \param[in] region Region of the output image.
       * \param[in] spacing Spacing of the output image.
       * \param[in] origin Origin of the output image.
       *
       */
      itkVolumeType::Pointer labelObjectToImage(const SLO::Pointer object,
                                                const RegionType &region,
                                                const SpacingType &spacing,
                                                const OriginType &origin);

    private:
      bool                   m_useSLIC;   /** true if the filter uses SLIC information, false otherwise. */
      double                 m_threshold; /** threshold value.                                           */
      Extensions::StackSLIC *m_slic;      /** Core factory object for SLIC extension creation.           */
      QStringList            m_errors;    /** Error message or empty if filter ran successfully.         */
  };

  using SliceInterpolationFilterPtr = SliceInterpolationFilter *;
  using SliceInterpolationFilterSPtr = std::shared_ptr<SliceInterpolationFilter>;

} /* namespace ESPINA */

#endif /* FILTERS_SLICEINTERPOLATIONFILTER_H_ */
