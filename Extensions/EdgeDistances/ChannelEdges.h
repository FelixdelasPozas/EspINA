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

#ifndef ESPINA_CHANNEL_EDGES_H
#define ESPINA_CHANNEL_EDGES_H

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/Analysis/Extension.h>
#include <Core/Utils/Spatial.h>
#include "AdaptiveEdgesCreator.h"
#include "EdgesAnalyzer.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <QMutex>

namespace EspINA
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
    explicit ChannelEdges(SchedulerSPtr   scheduler = SchedulerSPtr(),
                          const InfoCache &cache    = InfoCache(),
                          const State     &state    = State());
    virtual ~ChannelEdges();

    virtual Type type() const
    { return TYPE; }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual InfoTagList availableInformations() const
    { return InfoTagList(); }

    void setUseDistanceToBounds(bool value);

    bool useDistanceToBounds() const;

    /**
     * \brief Return the image region that excludes slice margin voxels
     */
    itkVolumeType::RegionType sliceRegion(unsigned int slice) const;

    void distanceToBounds(SegmentationPtr segmentation, Nm distances[6]) const;

    void distanceToEdges(SegmentationPtr segmentation, Nm distances[6]);

    vtkSmartPointer<vtkPolyData> channelEdges();

    Nm computedVolume();

    void setBackgroundColor(int value);

    int backgroundColor() const;

    void setThreshold(int value);

    int threshold() const;

  protected:
    virtual void onExtendedItemSet(Channel* item);

    virtual QVariant cacheFail(const QString& tag) const
    { return QVariant(); }

  private:
    void initializeEdges();

    void analyzeChannel();

    void computeAdaptiveEdges();

    void loadEdgesCache();

    void loadFacesCache();

  private slots:
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

    // build a surface for each face the first time they're needed
    void computeSurfaces();

    friend class AdaptiveEdgesCreator;
    friend class EdgesAnalyzer;
  };

  using ChannelEdgesPtr  = ChannelEdges *;
  using ChannelEdgesSPtr = std::shared_ptr<ChannelEdges>;

  ChannelEdgesPtr  channelEdgesExtension(ChannelExtensionPtr extension);
  ChannelEdgesSPtr channelEdgesExtension(ChannelPtr channel);

}// namespace EspINA

#endif // ESPINA_CHANNEL_EDGES_H
