/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "analysis_testing_support.h"

using namespace std;

bool EspINA::Testing::checkAnalysisExpectedElements(Analysis& analysis,
                                                    int numSamples,
                                                    int numChannels,
                                                    int numSegmentations,
                                                    int numFilters,
                                                    int numContentRelations,
                                                    int numRelations)
{
  bool error = false;

  if (analysis.samples().size() != numSamples) {
    cerr << "Unexpected number of samples in analysis" << endl;
    error = true;
  }

  if (analysis.channels().size() != numChannels) {
    cerr << "Unexpected number of channels in analysis" << endl;
    error = true;
  }

  if (analysis.segmentations().size() != numSegmentations) {
    cerr << "Unexpected number of segmentations in analysis" << endl;
    error = true;
  }

  if (analysis.content()->vertices().size() != numSamples + numChannels + numSegmentations + numFilters) {
    cerr << "Unexpected number of vertices in analysis content" << endl;
    error = true;
  }

  if (analysis.content()->edges().size() != numContentRelations) {
    cerr << "Unexpected number of edges in analysis content" << endl;
    error = true;
  }

  if (analysis.relationships()->vertices().size() != numSamples + numChannels + numSegmentations) {
    cerr << "Unexpected number of vertices in analysis relationships" << endl;
    error = true;
  }

  if (!analysis.relationships()->edges().size() != numRelations) {
    cerr << "Unexpected number of edges in analysis relationships" << endl;
    error = true;
  }

  return error;
}
