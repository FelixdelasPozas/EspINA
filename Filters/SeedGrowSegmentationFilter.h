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

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include "Core/Analysis/Filter.h"
#include <Core/Utils/BinaryMask.h>
#include "Core/EspinaTypes.h"

class vtkImageData;
class vtkConnectedThresholdImageFilter;

namespace ESPINA
{
  class SeedGrowSegmentationFilter
  : public Filter
  {
  public:
    explicit SeedGrowSegmentationFilter(InputSList inputs, Type type, SchedulerSPtr scheduler);

    virtual void restoreState(const State& state);

    virtual State state() const;

    void setLowerThreshold(int th);

    int lowerThreshold() const;

    void setUpperThreshold(int th);

    int upperThreshold() const;

    // Convenience method to set symmetrical lower/upper thresholds
    void setThreshold(int th)
    {
      setLowerThreshold(th);
      setUpperThreshold(th);
    };

    void setSeed(const NmVector3& seed);
    NmVector3 seed() const;

    void setROI(const ROISPtr roi);

    template<typename T>
    BinaryMask<T> roi() const;

    void setClosingRadius(int radius);

    int closingRadius();

  protected:
    virtual Snapshot saveFilterSnapshot() const;

    virtual bool needUpdate() const;

    virtual bool needUpdate(Output::Id id) const;

    virtual void execute();

    virtual void execute(Output::Id id);

    virtual bool ignoreStorageContent() const;

    virtual bool invalidateEditedRegions();

//   private:
//     Bounds minimalBounds(itkVolumeType::Pointer image) const;

  private:
    int       m_lowerTh, m_prevLowerTh;
    int       m_upperTh, m_prevUpperTh;
    NmVector3 m_seed,    m_prevSeed;
    int       m_radius,  m_prevRadius;
    bool      m_usesROI;
    ROISPtr   m_roi;
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_FILTER_H
