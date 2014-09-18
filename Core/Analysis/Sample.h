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

#ifndef ESPINA_SAMPLE_H
#define ESPINA_SAMPLE_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include "Core/Utils/Bounds.h"
#include <Core/Utils/NmVector3.h>
#include "Core/Analysis/NeuroItem.h"

namespace ESPINA
{
  using SampleSList = QList<SampleSPtr>;

  /** \brief Sample
   *
   */
  class EspinaCore_EXPORT Sample
  : public NeuroItem
  {
  public:
    const static RelationName CONTAINS;

    /* \brief Sample class constructor.
     * \param[in] name, name of the sample.
     *
     */
    explicit Sample(const QString& name=QString());

    /* \brief Sample class destructor.
     *
     */
    virtual ~Sample();

    /* \brief Implements Persistent::restoreState().
     *
     */
    virtual void restoreState(const State& state);

    /* \brief Implements Persistent::state().
     *
     */
    virtual State state() const;

    /* \brief Implements Persistent::snapshot().
     *
     */
    virtual Snapshot snapshot() const;

    /* \brief Implements Persistent::unload().
     *
     */
    virtual void unload();

    /* \brief Sets the position of the sample.
     * \param[in] point, position.
     *
     */
    void setPosition(const NmVector3& point);

    /* \brief Returns the position of the sample.
     *
     */
    NmVector3 position() const;

    /** \brief Set the spatial bounds in nm of the Sample in the Analysis frame reference.
     *
     */
    void setBounds(const Bounds& bounds)
    { m_bounds = bounds; }

    /** \brief Return the spatial bounds in nm of the Sample in the Analysis frame reference.
     *
     */
    Bounds bounds() const
    { return m_bounds; }

  private:
    Bounds  m_bounds;
  };
}// namespace ESPINA

#endif // ESPINA_SAMPLE_H
