/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_EDGE_DISTANCE_H
#define ESPINA_EDGE_DISTANCE_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extension.h>
#include <Core/Utils/Spatial.h>

// Qt
#include <QMutex>

namespace ESPINA
{

  class EspinaExtensions_EXPORT EdgeDistance
  : public SegmentationExtension
  {
  public:
    static const Type TYPE;

    static const Key LEFT_DISTANCE;
    static const Key TOP_DISTANCE;
    static const Key FRONT_DISTANCE;
    static const Key RIGHT_DISTANCE;
    static const Key BOTTOM_DISTANCE;
    static const Key BACK_DISTANCE;

    /** \brief EdgeDistance class constructor.
     *
     */
    explicit EdgeDistance(const InfoCache &cache = InfoCache(),
                          const State     &state = State());

    /** \brief EdgeDistance class destructor.
     *
     */
    virtual ~EdgeDistance();

    virtual Type type() const
    { return TYPE; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual InformationKeyList availableInformation() const;

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    /** \brief Returns the distances as numerical values in the parameter.
     * \param[out] distances.
     *
     */
    void edgeDistance(Nm distances[6]) const;

  protected:
    virtual QVariant cacheFail(const InformationKey& key) const;

    virtual void onExtendedItemSet(Segmentation* segmentation);

  private:
    /** \brief Updated the distances of the extended item to the edges of its channel.
     *
     */
    void updateDistances() const;

  private:
    mutable QMutex m_mutex;

    friend class ChannelEdges;
  };

  using EdgeDistancePtr  = EdgeDistance *;
  using EdgeDistanceSPtr = std::shared_ptr<EdgeDistance>;

  /** \brief Returns the extension as an EdgeDistance raw pointer.
   * \param[in] extension, segmentation extension raw pointer.
   *
   */
  EdgeDistancePtr edgeDistance(SegmentationExtensionPtr extension);

}// namespace ESPINA

#endif // ESPINA_EDGE_DISTANCE_H
