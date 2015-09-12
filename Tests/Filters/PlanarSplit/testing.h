/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef PLANAR_SPLIT_TESTING_H
#define PLANAR_SPLIT_TESTING_H

#include <Core/Types.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/Analysis/Segmentation.h>
#include <testing_support_channel_input.h>

namespace ESPINA
{
  class TestFilterFactory
  : public FilterFactory
  {
  public:
    virtual FilterTypeList providedFilters() const;

    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler) const;
  };

  SegmentationSList gls_split(ChannelSPtr channel);

  bool dilate(SegmentationSPtr segmentation);


  bool checkSplitBounds(SegmentationSPtr source, SegmentationSPtr split1, SegmentationSPtr split2);

  AnalysisSPtr loadAnalyisis(QFileInfo file, CoreFactorySPtr factory);

  bool checkSegmentations(AnalysisSPtr analysis, int number);

  bool checkValidData(SegmentationSPtr segmentation, int numVolumeEditedRegions);

  bool checkSpacingChange(const NmVector3 &lhs, const NmVector3 &rhs);

  template<typename T>
  bool checkFilterType(SegmentationSPtr segmentation)
  {
    auto filter = dynamic_cast<T *>(segmentation->output()->filter());

    bool error = !filter;

    if (error)
    {
      std::cerr << "Unexpected Filter Type " << filter->type().toStdString() << std::endl;
    }

    return error;
  }
}

#endif // PLANAR_SPLIT_TESTING_H
