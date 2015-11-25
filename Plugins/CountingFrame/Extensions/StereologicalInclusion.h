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

#ifndef ESPINA_STEREOLOGICAL_INCLUSION_H
#define ESPINA_STEREOLOGICAL_INCLUSION_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"

// ESPINA
#include <Core/Analysis/Extension.h>
#include <Core/Utils/Bounds.h>

class vtkPoints;
class vtkPolyData;

namespace ESPINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT StereologicalInclusion
    : public SegmentationExtension
    {
        Q_OBJECT

        static const QString FILE;

      public:
        static const Type           TYPE;
        static const InformationKey TOUCH_EDGES;

        InformationKey cfKey(CountingFrame *cf) const;

      public:
        /** \brief StereologicalInclusion class constructor.
         * \param[in] infoCache information cache object.
         *
         */
        explicit StereologicalInclusion(const InfoCache &infoCache = InfoCache());

        /** \brief StereologicalInclusion class virtual destructor.
         *
         */
        virtual ~StereologicalInclusion();

        virtual Type type() const
        { return TYPE; }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual State state() const;

        virtual Snapshot snapshot() const;

        virtual TypeList dependencies() const;

        virtual bool validCategory(const QString& classificationName) const
        { return true; }

        virtual InformationKeyList availableInformation() const;

        virtual QString toolTipText() const;

        /** \brief Adds the given CF to the list of CFs to check for inclusion.
         * \param[in] cf counting frame object pointer.
         *
         */
        void addCountingFrame(CountingFrame *cf);

        /** \brief Removes the given CF from the list of CFs to check for inclusion.
         * \param[in] cf counting frame object pointer.
         *
         */
        void removeCountingFrame(CountingFrame *cf);

        /** \brief Returns true if has CFs to check for inclusion.
         *
         */
        bool hasCountingFrames() const;

        /** \brief Returns true if the segmentation is excluded at least by one CF.
         *
         */
        bool isExcluded();

        /** \brief Returns true if the segmentation is at the edge of the channel.
         *
         */
        bool isOnEdge() const;

      protected:
        virtual QVariant cacheFail(const InformationKey& tag) const;

        virtual void onExtendedItemSet(Segmentation *segmentation);

      public slots:
        /** \brief Evaluates the inclusion in the given counting frame.
         * \param[in] cf counting frame object pointer.
         *
         */
        void evaluateCountingFrame(CountingFrame *cf);

        /** \brief Evaluates the inclusion in the list of stored CFs.
         *
         */
        void evaluateCountingFrames();

      private:
        /** \brief Returns true if the segmentation is excluded by the given CF.
         * \param[in] cf counting frame object pointer.
         *
         */
        bool isExcludedByCountingFrame(CountingFrame *cf);

        /** \brief Returns true if the segmentation collides with the given bounds.
         *
         */
        bool isRealCollision(const Bounds& collisionBounds);

        /** \brief Returns the list of CFs defined for the segmentation's sample.
         *
         */
        void checkSampleCountingFrames();

      private:
        bool m_isInitialized;                          /** true if the extension has been initialized.              */
        bool m_isUpdated;                              /** true if the extension data is up to date.                */

        QMutex m_mutex;                                /** lock for extension data computation.                     */
        bool   m_isExcluded;                           /** true if the segmentation is excluded by at least one CF. */
        QMap<CountingFrame *, bool>   m_exclusionCFs;  /** maps CF pointer - exclusion information.                 */
        QMap<CountingFrame::Id, bool> m_excludedByCF;  /** maps CF::id - exclusion information.                     */
    };

    using StereologicalInclusionPtr  = StereologicalInclusion *;
    using StereologicalInclusionSPtr = std::shared_ptr<StereologicalInclusion>;

    StereologicalInclusionSPtr stereologicalInclusion(SegmentationExtensionSPtr extension);

  } // namespace CF
} // namespace ESPINA

#endif // STEREOLOGICALINCLUSION_H
