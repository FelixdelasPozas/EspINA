/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_REPRESENTATION_POOL_H
#define ESPINA_REPRESENTATION_POOL_H

#include <Core/Utils/NmVector3.h>
#include <GUI/Representations/RepresentationsState.h>
#include "RepresentationPipeline.h"

#include <memory>

namespace ESPINA
{
  class RepresentationPool
  {
  public:
    virtual ~RepresentationPool() {}

    void setState(RepresentationsStateSPtr state);

    /** \brief Updates pool representation pipelines to the given position
     *
     */
    virtual void setCrosshair(NmVector3 position) = 0;

    /** \brief Returns whether all pipeline representations are set to the
     *         current position or not
     *
     */
    virtual bool isReady() const = 0;

    /** \brief Returns all representation pipelines in the pool
     *
     */
    virtual RepresentationPipelineSList representationPipelines() = 0;

    /** \brief Returns visible representation pipelines in the pool
     *
     */
    virtual RepresentationPipelineSList visibleRepresentationPipelines() = 0;

    /** \brief Returns invisible representation pipelines in the pool
     *
     */
    virtual RepresentationPipelineSList invisibleRepresentationPipelines() = 0;

    /** \brief Increment the number of active managers using this pool
     *
     */
    void incrementActiveManagers(); // manage?

    /** \brief Decrement the number of active managers using this pool
     *
     */
    void decrementActiveManagers(); // release?

  protected:
    explicit RepresentationPool();

    /** \brief Returns whether the pool representations are displayed by
     *         at least one representation manager
     */
    bool isBeingUsed() const;

  private:
    RepresentationsStateSPtr m_state;

    unsigned m_numActiveManagers;
  };

  using RepresentationPoolSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationPoolSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
