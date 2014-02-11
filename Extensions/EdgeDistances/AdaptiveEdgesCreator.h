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


#ifndef ESPINA_ADAPTIVE_EDGE_CREATOR_H
#define ESPINA_ADAPTIVE_EDGE_CREATOR_H

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/MultiTasking/Task.h>

namespace EspINA
{
  class ChannelEdges;

  class EspinaExtensions_EXPORT AdaptiveEdgesCreator
  : public Task
  {
  public:
    explicit AdaptiveEdgesCreator(ChannelEdges *extension,
                                  SchedulerSPtr scheduler = SchedulerSPtr());
    virtual ~AdaptiveEdgesCreator();

  protected:
    virtual void run();

  private:
    ChannelEdges *m_extension;
};

  using AdaptiveEdgesCreatorPtr  = AdaptiveEdgesCreator *;
  using AdaptiveEdgesCreatorSPtr = std::shared_ptr<AdaptiveEdgesCreator>;

}// namespace EspINA

#endif // ESPINA_ADAPTIVE_EDGE_CREATOR_H
