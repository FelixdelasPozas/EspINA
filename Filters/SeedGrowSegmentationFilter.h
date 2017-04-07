/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ESPINA_SEED_GROW_SEGMENTATION_FILTER_H
#define ESPINA_SEED_GROW_SEGMENTATION_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include "Core/Analysis/Filter.h"
#include <Core/Utils/BinaryMask.hxx>
#include "Core/Types.h"

class vtkImageData;
class vtkConnectedThresholdImageFilter;

namespace ESPINA
{
  class EspinaFilters_EXPORT SeedGrowSegmentationFilter
  : public Filter
  {
      Q_OBJECT
    public:
      /** \brief SeedGrowSegmentationFilter class constructor.
       * \param[in] inputs list of input smart pointers.
       * \param[in] type SeedGrowSegmentationFilter type.
       * \param[in] scheduler scheduler smart pointer.
       *
       */
      explicit SeedGrowSegmentationFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler);

      /** \brief SeedGrowSegmentationFilter class virtual destructor.
       *
       */
      virtual ~SeedGrowSegmentationFilter()
      {}

      virtual void restoreState(const State& state);

      virtual State state() const;

      virtual void changeSpacing(const NmVector3 &origin, const NmVector3 &spacing);

      /** \brief Sets the lower value of the threshold.
       * \param[in] th lower threshold value.
       *
       */
      void setLowerThreshold(int th);

      /** \brief Returns the lower threshold value.
       *
       */
      int lowerThreshold() const;

      /** \brief Sets the upper threshold value.
       * \param[in] th upper threshold value.
       *
       */
      void setUpperThreshold(int th);

      /** \brief Returns the upper threshold value.
       *
       */
      int upperThreshold() const;

      /** \brief Convenience method to set symmetrical lower/upper thresholds.
       * \param[in] th threshold value.
       *
       */
      void setThreshold(int th)
      {
        setLowerThreshold(th);
        setUpperThreshold(th);
      };

      /** \brief Sets the seed point.
       * \param[in] seed seed point.
       *
       */
      void setSeed(const NmVector3& seed);

      /** \brief Returns the seed point.
       *
       */
      NmVector3 seed() const;

      /** \brief Sets the region of interest to constrain the application of the filter.
       * \param[in] roi ROI object smart pointer.
       *
       */
      void setROI(const ROISPtr roi);

      /** \brief Returns the ROI of the filter.
       *
       */
      ROISPtr roi() const;

      /** \brief Sets the radious for the closing morphological operation.
       * \param[in] radious close filter radius.
       *
       */
      void setClosingRadius(int radius);

      /** \brief Returns the closing filter radius.
       *
       */
      int closingRadius() const;

      /** \brief Returns true if the resulting segmentation touches the used ROI.
       *
       */
      bool isTouchingROI() const
      { return m_touchesROI; };

      /** \brief Forces filter execution even if its parameters haven't changed
       *
       */
      void forceUpdate()
      { m_forceUpdate = true; }

    signals:
      void seedModified(NmVector3 seed);
      void thresholdModified(int, int);
      void radiusModified(int radius);
      void roiModified(ROISPtr roi);

    protected:
      virtual Snapshot saveFilterSnapshot() const;

      virtual bool needUpdate() const;

      virtual void execute();

      virtual bool ignoreStorageContent() const;

    private:
      /** \brief Helper method that returns true if the segmentation touches the ROI.
       *
       */
       bool computeTouchesROIValue() const;

       /** \brief Returns the ROI id for this filter.
        *
        */
       QString roiId() const
       { return "sgs"; }

    private:
      int       m_lowerTh, m_prevLowerTh; /** lower threshold, settings lower threshold.*/
      int       m_upperTh, m_prevUpperTh;
      NmVector3 m_seed,    m_prevSeed;
      int       m_radius,  m_prevRadius;

      mutable bool    m_hasROI;  // roi() const
      mutable ROISPtr m_ROI;     // roi() const
      mutable ROIPtr  m_prevROI; // roi() const

      bool      m_touchesROI;
      bool      m_forceUpdate;
  };

  using SeedGrowSegmentationFilterSPtr = std::shared_ptr<SeedGrowSegmentationFilter>;
} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_FILTER_H
