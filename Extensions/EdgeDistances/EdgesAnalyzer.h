/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricData.h>

namespace EspINA
{
  class ChannelEdges;

  class EspinaExtensions_EXPORT EdgesAnalyzer
  : public Task
  {
  public:
    explicit EdgesAnalyzer(ChannelEdges *extension,
                           SchedulerSPtr scheduler = SchedulerSPtr());
    virtual ~EdgesAnalyzer();

  protected:
    virtual void run();

  private:
    void analyzeEdge(DefaultVolumetricDataSPtr volume, const Bounds &edgeBounds);

  private:
    int m_useDistanceToBounds;
    int m_bgIntensity;

    ChannelEdges *m_extension;
};

  using EdgesAnalyzerPtr  = EdgesAnalyzer *;
  using EdgesAnalyzerSPtr = std::shared_ptr<EdgesAnalyzer>;

}// namespace EspINA

#endif // ESPINA_EDGES_ANALYZER_H
