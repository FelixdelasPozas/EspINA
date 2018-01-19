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

#ifndef ESPINA_CHANNEL_EDGES_H
#define ESPINA_CHANNEL_EDGES_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include "AdaptiveEdgesCreator.h"
#include "EdgesAnalyzer.h"
#include <Core/Utils/Spatial.h>
#include <Core/Analysis/Extensions.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt
#include <QMutex>
#include <QWaitCondition>

namespace ESPINA
{
  namespace Extensions
  {
    class ChannelEdgesFactory;
    class AdaptiveEdgesCreator;
    class EdgesAnalyzer;

    class EspinaExtensions_EXPORT ChannelEdges
    : public Core::StackExtension
    {
      static const QString EDGES_FILE;
      static const QString FACES_FILE;

      Q_OBJECT
    public:
      static const Type TYPE;

    public:
      /** \brief ChannelEdges class destructor.
       *
       */
      virtual ~ChannelEdges();

      virtual Type type() const
      { return TYPE; }

      virtual bool invalidateOnChange() const
      { return true; }

      virtual State state() const;

      virtual Snapshot snapshot() const;

      virtual TypeList dependencies() const
      { return TypeList(); }

      virtual InformationKeyList availableInformation() const
      { return InformationKeyList(); }

      QString snapshotName(const QString &file) const;

      virtual void invalidate() override;

      /** \brief Return the image region that excludes slice margin voxels.
       *
       */
      itkVolumeType::RegionType sliceRegion(unsigned int slice) const;

      /** \brief Returns the distances in Nm from the segmentations to the bounds of the channel.
       * \param[in] segmentation, segmentation raw pointer.
       * \param[out] distances, distances in each direction.
       *
       */
      void distanceToBounds(SegmentationPtr segmentation, Nm distances[6]) const;

      /** \brief Returns the distances in Nm from the segmentations to the edges of the channel.
       * \param[in] segmentation to measure distances
       * \param[out] distances in each direction.
       *
       */
      void distanceToEdges(SegmentationPtr segmentation, Nm distances[6]);

      /** \brief Returns the vtkPolyData that define the edges of the channel.
       *
       */
      vtkSmartPointer<vtkPolyData> channelEdges();

      virtual QString toolTipText() const
      { return tr("Channel Edges"); }

      /** \brief Return the volume un Nm^3.
       *
       */
      Nm computedVolume();

      /** \brief Sets the values of the stack edges.
       * \param[in] useBounds true to use stack bounds and false otherwise.
       * \param[in] value color intensity value.
       * \param[in] threshold threshold value.
       *
       * NOTE: if (useBounds == false) and (color == -1) it forces a re-evaluation of the values.
       *
       */
      void setAnalisysValues(bool useBounds, int color, int threshold);

      /** \brief Returns the channel background color.
       *
       */
      int backgroundColor() const;

      /** \brief Returns the threshold value.
       *
       */
      int threshold() const;

      /** \brief Returns the "use distance to bounds" flag.
       *
       */
      bool useDistanceToBounds() const;

      /** \brief Returns true if the given point is near the edge given the tolerance distance.
       * \param[in] point Point 3D coordinates.
       * \param[in] tolerance Maximum distance from the edge to consider the point near it.
       *
       */
      bool isPointOnEdge(const NmVector3 point, const Nm tolerance);

      /** \brief Returns true if the edges have been computed and are available.
       *
       */
      bool areEdgesAvailable() const
      { return m_hasCreatedEdges; }

    protected:
      virtual void onExtendedItemSet(Channel* item);

      virtual QVariant cacheFail(const InformationKey& tag) const
      { return QVariant(); }

    private:
      /** \brief ChannelEdges class constructor.
       * \param[in] scheduler, scheduler smart pointer.
       * \parma[in] cache, cache object.
       * \param[in] state, state object.
       *
       */
      explicit ChannelEdges(SchedulerSPtr   scheduler = SchedulerSPtr(),
                            const InfoCache &cache    = InfoCache(),
                            const State     &state    = State());

      /** \brief Loads the edges from the cache and computes the adaptive edges.
       *
       */
      void initializeEdges();

      /** \brief Launches the edges analizer task.
       *
       */
      void analyzeChannel();

      /** \brief Computes the channel's adaptive edges.
       *
       */
      void computeAdaptiveEdges();

      /** \brief Loads edges and faces polydatas from disk.
       *
       */
      void loadEdgesData();

      /** \brief Internal implementation of invalidate() without sending a signal.
       *
       */
      void invalidateResults();

      void checkAnalysisData() const;
      void checkEdgesData();

    private:
      /** \brief Helper method to create the rectangular edges vtkPolyData.
       * \param[in] bounds limits of the rectangular region.
       *
       */
      void createRectangularRegion(const Bounds &bounds);

      mutable std::atomic<bool> m_hasAnalizedChannel;
      mutable std::atomic<bool> m_hasCreatedEdges;

      mutable QWaitCondition m_analisysWait;        /** wait condition for EdgesAnalyzer task.                                                    */
      mutable QMutex         m_analysisResultMutex; /** protects use distances, background and threshold values.                                  */
      mutable QWaitCondition m_edgesTask;           /** wait condition for AdaptiveEdges task.                                                    */
      mutable QMutex         m_edgesResultMutex;    /** barrier signaling end of edges computation.                                               */
      mutable QReadWriteLock m_dataMutex;           /** protects class internal data.                                                             */
      QMutex                 m_distanceMutex;       /** protects edges polydata during distance to edges computation.                             */

      bool   m_useDistanceToBounds;                 /** true to use the distance to the stack bounds, false otherwise.                            */
      int    m_backgroundColor;                     /** background color intensity value.                                                         */
      int    m_threshold;                           /** background color threshold value.                                                         */
      Nm     m_computedVolume;                      /** measure in nm^3 of the volume enclosed in the computed edges.                             */

      bool   m_invalidated;                         /** true if the values have been invalidated and needs to be computed again, false otherwise. */

      AdaptiveEdgesCreatorSPtr m_edgesCreator;      /** task that creates the edges polydata.                                                     */
      EdgesAnalyzerSPtr        m_edgesAnalyzer;     /** task that analyzes border values and threshold.                                           */

      vtkSmartPointer<vtkPolyData> m_edges;         /** stack edges polydata.                                                                     */
      vtkSmartPointer<vtkPolyData> m_faces[6];      /** edges faces polydatas.                                                                    */

      SchedulerSPtr m_scheduler;                    /** application task scheduler.                                                               */

      friend class AdaptiveEdgesCreator;
      friend class EdgesAnalyzer;
      friend class ChannelEdgesFactory;
    };

    using ChannelEdgesPtr  = ChannelEdges *;
    using ChannelEdgesSPtr = std::shared_ptr<ChannelEdges>;
  } // namespace Extensions
}// namespace ESPINA

#endif // ESPINA_CHANNEL_EDGES_H
