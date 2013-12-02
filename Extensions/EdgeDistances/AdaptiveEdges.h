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

#ifndef ESPINA_ADAPTIVE_EDGES_H
#define ESPINA_ADAPTIVE_EDGES_H

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/Analysis/Extensions/ChannelExtension.h>
#include <Core/Utils/Spatial.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <QMutex>

namespace EspINA
{
  class EspinaExtensions_EXPORT AdaptiveEdges
  : public ChannelExtension
  {
    static const QString EXTENSION_FILE;
    static const QString EDGES_FILE;
    static const QString FACES_FILE;

  public:
    static const ModelItem::ArgumentId EDGETYPE;

    static const Type TYPE;

  public:
    explicit AdaptiveEdges(bool useAdaptiveEdges = false,
                           int backgroundColor   = 0,
                           int threshold         = 50);
    virtual ~AdaptiveEdges();

    virtual Type type() const
    { return TYPE; }

    void computeDistanceToEdge(SegmentationPtr seg);

    vtkSmartPointer<vtkPolyData> channelEdges();
    Nm computedVolume();

    bool usesAdaptiveEdges() const
    { return m_useAdaptiveEdges; }

    int backgroundColor() const
    { return m_backgroundColor; }

    int threshold() const
    { return m_threshold; }

  private:
    void computeAdaptiveEdges();
//     void loadEdgesCache(ChannelPtr channel);
// 
//     void loadFacesCache(ChannelPtr channel);
// 
// 
//     QString fileId(ChannelPtr channel) const;

  private:
    QMutex m_mutex;
    bool   m_useAdaptiveEdges;
    int    m_backgroundColor;
    int    m_threshold;

    // build a surface for each face the first time they're needed
    void ComputeSurfaces();

    friend class EdgeDetector;
  };

  using AdaptiveEdgesPtr  = AdaptiveEdges *;
  using AdaptiveEdgesSPtr = std::shared_ptr<AdaptiveEdges>;

  AdaptiveEdgesPtr EspinaExtensions_EXPORT adaptiveEdges(ChannelExtensionPtr extension);

}// namespace EspINA

#endif // ESPINA_ADAPTIVE_EDGES_H
