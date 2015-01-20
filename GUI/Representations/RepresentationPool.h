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

#include <memory>

namespace ESPINA
{
  class RepresentationPool
  {
  public:
    virtual ~RepresentationPool() {}

    /** \brief Notifies the cache to update its representation pipelines
     *         to the given position
     *
     */
    virtual void setCrosshair(NmVector3 position) = 0;

    /** \brief Returns whether all pipeline representations are set to the
     *         current position or not
     *
     */
    virtual bool isReady() const = 0;

    // TODO: Que debería devolver? Una lista de actores o sus pipeline asociados
    virtual QList<int> pipelineRepresentations() = 0;

    bool isBeingUsed() const;

    /** \brief Increment the number of active renderers using this cache (TODO)
     *
     */
    void incrementActiveRenderers();

    /** \brief Decrement the number of active renderers using this cache (TODO)
     *
     */
    void decrementActiveRenderers();

  protected:
    explicit RepresentationPool();

  private:
    unsigned m_numActiveRenderers;

  };

  using RepresentationCacheSPtr  = std::shared_ptr<RepresentationPool>;
  using RepresentationPoolSList = QList<RepresentationCacheSPtr>;
} // namespace ESPINA

#endif // ESPINA_REPRESENTATION_POOL_H
