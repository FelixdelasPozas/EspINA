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

#ifndef ADAPTIVEEDGES_H
#define ADAPTIVEEDGES_H

#include "EspinaCore_Export.h"

#include "Core/Extensions/ChannelExtension.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <QMutex>

namespace EspINA
{
  static const ModelItem::ExtId AdaptiveEdgesID = "AdaptiveEdges";

  class EspinaCore_EXPORT AdaptiveEdges
  : public Channel::Extension
  {
    static const QString EXTENSION_FILE;
    static const QString EDGES_FILE;
    static const QString FACES_FILE;

    struct ExtensionData
    {
      ExtensionData() 
      : ComputedVolume(0)
      , UseAdaptiveEdges(false)
      , BackgroundColor(0)
      , Threshold(50)
      {}

	  bool operator==(const ExtensionData& other) const
	  {
		  bool retVal = (ComputedVolume == other.ComputedVolume);
		  retVal |= (UseAdaptiveEdges == other.UseAdaptiveEdges);
		  retVal |= (BackgroundColor == other.BackgroundColor);
		  retVal |= (Threshold == other.Threshold);
		  retVal |= (Edges == other.Edges);
		  for (int i = 0; i < 6; ++i)
			  retVal |= (Faces[i] == other.Faces[i]);

		  return retVal;
	  }

      Nm   ComputedVolume;
      bool UseAdaptiveEdges;
      int BackgroundColor;
      int Threshold;

      vtkSmartPointer<vtkPolyData> Edges;
      vtkSmartPointer<vtkPolyData> Faces[6];
    };

    typedef Cache<ChannelPtr, ExtensionData> ExtensionCache;

    static ExtensionCache s_cache;

  public:
    static const ModelItem::ArgumentId EDGETYPE;

    explicit AdaptiveEdges(bool useAdaptiveEdges = false, int backgroundColor = 0, int threshold = 50);
    virtual ~AdaptiveEdges();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const
    {return Channel::Extension::dependencies();}

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }
//           || file.startsWith(EDGES_FILE)
//           || file.startsWith(FACES_FILE); }

    virtual void loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model);

    virtual bool saveCache(Snapshot &snapshot);

    virtual Channel::ExtensionPtr clone();

    virtual void initialize();

    virtual void invalidate(ChannelPtr channel = NULL);

    void computeDistanceToEdge(SegmentationPtr seg);

    vtkSmartPointer<vtkPolyData> channelEdges();
    Nm computedVolume();

    bool usesAdaptiveEdges() { return m_useAdaptiveEdges; }
    int backgroundColor() { return m_backgroundColor; }
    int threshold() { return m_threshold; }

  private:
    void computeAdaptiveEdges();

    void loadEdgesCache(ChannelPtr channel);

    void loadFacesCache(ChannelPtr channel);

    ChannelPtr findChannel(const QString &id,
                           int outputId,
                           const QDir &tmpDir,
                           IEspinaModel *model);

    QString fileId(ChannelPtr channel) const;

  private:
    QMutex m_mutex;
    bool   m_useAdaptiveEdges;
    int    m_backgroundColor;
    int    m_threshold;

    // build a surface for each face the first time they're needed
    void ComputeSurfaces();

    friend class EdgeDetector;
  };

  typedef AdaptiveEdges *AdaptiveEdgesPtr;
  typedef boost::shared_ptr<AdaptiveEdgesPtr> AdaptiveEdgesSPtr;

  AdaptiveEdgesPtr adaptiveEdgesPtr(Channel::ExtensionPtr extension);

}// namespace EspINA

#endif // ADAPTIVEEDGES_H
