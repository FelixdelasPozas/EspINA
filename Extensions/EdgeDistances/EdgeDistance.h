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
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Utils/Spatial.h>

// Qt
#include <QMutex>

namespace ESPINA
{
  class CoreFactory;

  namespace Extensions
  {
    class ChannelEdges;
    class EdgeDistanceFactory;

    /** \class EdgeDistance
     * \brief Segmentation extension that provides the distance to the edges of the channel of the segmentation.
     *
     */
    class EspinaExtensions_EXPORT EdgeDistance
    : public Core::SegmentationExtension
    {
      public:
        static const Type TYPE;

        static const Key LEFT_DISTANCE;
        static const Key TOP_DISTANCE;
        static const Key FRONT_DISTANCE;
        static const Key RIGHT_DISTANCE;
        static const Key BOTTOM_DISTANCE;
        static const Key BACK_DISTANCE;
        static const Key TOUCH_EDGES;

        /** \brief EdgeDistance class destructor.
         *
         */
        virtual ~EdgeDistance()
        {}

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

        virtual bool validData(const OutputSPtr output) const
        { return hasMeshData(output) || hasSkeletonData(output); }

        /** \brief Returns the distances as numerical values in the parameter.
         * \param[out] distances.
         *
         */
        void edgeDistance(Nm distances[6]) const;

        virtual QString toolTipText() const;

      protected:
        virtual QVariant cacheFail(const InformationKey& key) const;

        virtual void onExtendedItemSet(Segmentation* segmentation);

      private:
        /** \brief EdgeDistance class constructor.
         * \param[in] cache InfoCache object.
         * \param[in] state extension's state.
         *
         */
        explicit EdgeDistance(CoreFactory     *factory,
                              const InfoCache &cache = InfoCache(),
                              const State     &state = State());

        /** \brief Updated the distances of the extended item to the edges of its channel.
         *
         */
        void updateDistances() const;

        /** \brief Returns true if the segmentation is at the edge of the channel.
         *
         */
        bool isOnEdge() const;

      private:
        mutable QMutex m_mutex;    /** mutex for data protection.                   */
        CoreFactory   *m_factory;  /** core factory to get/create edges extensions. */

        friend class ChannelEdges;
        friend class EdgeDistanceFactory;
    };

    using EdgeDistancePtr  = EdgeDistance *;
    using EdgeDistanceSPtr = std::shared_ptr<EdgeDistance>;
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_EDGE_DISTANCE_H
