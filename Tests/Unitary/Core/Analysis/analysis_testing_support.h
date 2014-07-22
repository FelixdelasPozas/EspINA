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

#ifndef ANALYSIS_TESTING_SUPPORT_H
#define ANALYSIS_TESTING_SUPPORT_H

#include <Core/Analysis/Filter.h>
#include <Core/MultiTasking/Scheduler.h>

namespace ESPINA {
  namespace Testing {
//     bool checkAnalysisContent(Analysis& analysis,
//                               int numSamples,
//                               int numChannels,
//                               int numSegmentations,
//                               int numFilters,
//                               int numRelations);

    bool checkAnalysisExpectedElements(Analysis& analysis,
                                       int numSamples,
                                       int numChannels,
                                       int numSegmentations,
                                       int numFilters,
                                       int numContentRelations,
                                       int numRelations);
  }
}

#endif // ANALYSIS_TESTING_SUPPORT_H