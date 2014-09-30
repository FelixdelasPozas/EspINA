/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ESPINA_EDGES_ANALYZER_H
#define ESPINA_EDGES_ANALYZER_H

// ESPINA
#include "Extensions/EspinaExtensions_Export.h"
#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  class ChannelEdges;

  class EspinaExtensions_EXPORT EdgesAnalyzer
  : public Task
  {
  public:
  	/** \brief EdgesAnalyzer class constructor.
  	 * \param[in] extension, ChannelEdges raw pointer.
  	 * \param[in] scheduler, scheduler smart pointer.
  	 *
  	 */
    explicit EdgesAnalyzer(ChannelEdges *extension,
                           SchedulerSPtr scheduler = SchedulerSPtr());

  	/** \brief EdgesAnalyzer class destructor.
  	 *
  	 */
    virtual ~EdgesAnalyzer();

  protected:
  	/** \brief Computes the edges of a channel.
  	 *
  	 */
    virtual void run();

  private:
  	/** \brief Analizes the edge of a volume.
  	 * \param[in] volume, volumetric volume smart pointer to analyze.
  	 * \param[in] edgeBounds, bounds of the edge of the volume.
  	 *
  	 */
    void analyzeEdge(DefaultVolumetricDataSPtr volume, const Bounds &edgeBounds);

  private:
    int m_useDistanceToBounds;
    int m_bgIntensity;

    ChannelEdges *m_extension;
};

  using EdgesAnalyzerPtr  = EdgesAnalyzer *;
  using EdgesAnalyzerSPtr = std::shared_ptr<EdgesAnalyzer>;

}// namespace ESPINA

#endif // ESPINA_EDGES_ANALYZER_H
