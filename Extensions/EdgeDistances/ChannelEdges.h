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
#include <Core/Analysis/Extension.h>
#include <Core/Utils/Spatial.h>
#include "AdaptiveEdgesCreator.h"
#include "EdgesAnalyzer.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt
#include <QMutex>

namespace ESPINA
{
  class AdaptiveEdgesCreator;
  class EdgesAnalyzer;

  class EspinaExtensions_EXPORT ChannelEdges
  : public ChannelExtension
  {
    static const QString EDGES_FILE;
    static const QString FACES_FILE;

    Q_OBJECT
  public:
    static const Type TYPE;

  public:
    /** \brief ChannelEdges class constructor.
     * \param[in] scheduler, scheduler smart pointer.
     * \parma[in] cache, cache object.
     * \param[in] state, state object.
     *
     */
    explicit ChannelEdges(SchedulerSPtr   scheduler = SchedulerSPtr(),
                          const InfoCache &cache    = InfoCache(),
                          const State     &state    = State());

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

    /** \brief Sets the "use distance to bounds" flag.
     * \param[in] value true to use the distance to bounds, false otherwise.
     *
     */
    void setUseDistanceToBounds(bool value);

    /** \brief Returns the "use distance to bounds" flag.
     *
     */
    bool useDistanceToBounds() const;

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

    /** \brief Return the volume un Nm^3.
     *
     */
    Nm computedVolume();

    /** \brief Sets the channel background color.
     *
     */
    void setBackgroundColor(int value);

    /** \brief Returns the channel background color.
     *
     */
    int backgroundColor() const;

    /** \brief Sets the threshold value.
     *
     */
    void setThreshold(int value);

    /** \brief Returns the threshold value.
     *
     */
    int threshold() const;

  protected:
    virtual void onExtendedItemSet(Channel* item);

    virtual QVariant cacheFail(const InformationKey& tag) const
    { return QVariant(); }

  private:
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

    /** \brief Loads edge values from cache.
     *
     */
    void loadEdgesCache();

    /** \brief Loads face values from cache.
     *
     */
    void loadFacesCache();

  private slots:
    /** \brief Perform operations after finishing the edges computation.
     *
     */
    void onChannelAnalyzed();

  private:
    mutable QReadWriteLock m_analysisResultMutex;

    mutable QReadWriteLock m_edgesMutex;
    mutable QReadWriteLock m_facesMutex;
    mutable QReadWriteLock m_edgesResultMutex;

    bool   m_useDistanceToBounds;
    int    m_backgroundColor;
    Nm     m_computedVolume;
    int    m_threshold;

    AdaptiveEdgesCreatorSPtr m_edgesCreator;
    EdgesAnalyzerSPtr        m_edgesAnalyzer;

    vtkSmartPointer<vtkPolyData> m_edges;
    vtkSmartPointer<vtkPolyData> m_faces[6];

    /** \brief Build a surface for each face the first time they're needed.
     *
     */
    void computeSurfaces();

    friend class AdaptiveEdgesCreator;
    friend class EdgesAnalyzer;
  };

  using ChannelEdgesPtr  = ChannelEdges *;
  using ChannelEdgesSPtr = std::shared_ptr<ChannelEdges>;

}// namespace ESPINA

#endif // ESPINA_CHANNEL_EDGES_H
