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

#ifndef ESPINA_ADAPTIVE_EDGE_CREATOR_H
#define ESPINA_ADAPTIVE_EDGE_CREATOR_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/MultiTasking/Task.h>

namespace ESPINA
{
  class ChannelEdges;

  class EspinaExtensions_EXPORT AdaptiveEdgesCreator
  : public Task
  {
  public:
    /** \brief AdaptiveEdgesCreator class constructor.
     * \param[in] extension ChannelEdges raw pointer.
     * \param[in] scheduler scheduler smart pointer.
     */
    explicit AdaptiveEdgesCreator(ChannelEdges *extension,
                                  SchedulerSPtr scheduler = SchedulerSPtr());

    /** \brief AdaptiveEdgesCreator class destructor.
     *
     */
    virtual ~AdaptiveEdgesCreator();

  protected:
    virtual void run();

  private:
    ChannelEdges *m_extension;
};

  using AdaptiveEdgesCreatorPtr  = AdaptiveEdgesCreator *;
  using AdaptiveEdgesCreatorSPtr = std::shared_ptr<AdaptiveEdgesCreator>;

}// namespace ESPINA

#endif // ESPINA_ADAPTIVE_EDGE_CREATOR_H
