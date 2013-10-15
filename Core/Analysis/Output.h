/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef OUTPUT_H
#define OUTPUT_H

#include "EspinaCore_Export.h"

#include <Core/EspinaRegion.h>
#include <Core/EspinaTypes.h>

#include <QMap>

class QDir;
class QDir;
namespace EspINA
{

  class Filter;

  class GraphicalRepresentation;
  typedef boost::shared_ptr<GraphicalRepresentation> GraphicalRepresentationSPtr;
  typedef QList<GraphicalRepresentationSPtr>         GraphicalRepresentationSList;

  typedef int FilterOutputId;

  class EspinaCore_EXPORT FilterOutput
  : public QObject
  {
    static EspinaTimeStamp s_tick;
    Q_OBJECT
  protected:
    static const int INVALID_OUTPUT_ID;

  public:
    typedef QString                         OutputRepresentationName;
    typedef QList<OutputRepresentationName> OutputRepresentationNameList;

    class EditedRegion
    {
    public:
      EditedRegion(int id, const OutputRepresentationName &name, const EspinaRegion &region)
      : Id(id), Name(name), Region(region){}
      virtual ~EditedRegion() {}

      int                      Id;
      OutputRepresentationName Name;
      EspinaRegion             Region;

      virtual bool dump(QDir           cacheDir,
                        const QString &regionName,
                        Snapshot      &snapshot) const = 0;
    };

    typedef boost::shared_ptr<EditedRegion> EditedRegionSPtr;
    typedef QList<EditedRegionSPtr>         EditedRegionSList;

    class OutputRepresentation;
    typedef boost::shared_ptr<OutputRepresentation> OutputRepresentationsPtr;
    typedef QList<OutputRepresentationsPtr>         OutputRepresentationsList;

  public:
    explicit FilterOutput(Filter               *filter = NULL,
                          const FilterOutputId &id     = INVALID_OUTPUT_ID);

    virtual bool isValid() const = 0;

    FilterOutputId id() const
    { return m_id; }

    bool isCached() const
    { return m_isCached; }

    void setCached(bool value)
    { m_isCached = value; }

    FilterPtr filter() const
    { return m_filter; }

    /// Convenience function to update a filter's output
    /// Some filters may execute partial updates
    void update();

    // TODO: Representation may have different bounds, in which case,
    // this function will be needed to represent the bounding box of all those regions
    virtual EspinaRegion region() const = 0;

    void addGraphicalRepresentation(GraphicalRepresentationSPtr prototype)
    { m_repPrototypes << prototype; }

    GraphicalRepresentationSList graphicalRepresentations() const
    { return m_repPrototypes; }

    void clearGraphicalRepresentations()
    { m_repPrototypes.clear(); }

    EspinaTimeStamp timeStamp()
    { return m_timeStamp; }

    // NOTE: Temporal solution to change output when filters are updated
    void updateModificationTime() 
    { m_timeStamp = s_tick++; }

  protected slots:
    void onRepresentationChanged();

  signals:
    void modified();

  protected:
    FilterOutputId m_id;
    bool           m_isCached; /// Whether output is used by a segmentation
    FilterPtr      m_filter;

    GraphicalRepresentationSList m_repPrototypes;

  private:
    EspinaTimeStamp m_timeStamp;
  };

  typedef FilterOutput                  * OutputPtr;
  typedef boost::shared_ptr<FilterOutput> OutputSPtr;
  typedef QList<OutputSPtr> OutputSList;

  class ChannelRepresentation;
  typedef boost::shared_ptr<ChannelRepresentation> ChannelRepresentationSPtr;

  class EspinaCore_EXPORT ChannelOutput
  : public FilterOutput
  {
  public:
    explicit ChannelOutput(Filter *filter = 0, const FilterOutputId &id = INVALID_OUTPUT_ID);

    virtual bool isValid() const;

    virtual EspinaRegion region() const;

    void setRepresentation(const OutputRepresentationName &name, ChannelRepresentationSPtr representation)
    { m_representations[name] = representation; }

    ChannelRepresentationSPtr representation(const OutputRepresentationName &name) const
    { return m_representations.value(name, ChannelRepresentationSPtr()); }

  private:
    QMap<OutputRepresentationName, ChannelRepresentationSPtr> m_representations;
  };

  typedef ChannelOutput                  * ChannelOutputPtr;
  typedef boost::shared_ptr<ChannelOutput> ChannelOutputSPtr;
  typedef QList<ChannelOutputSPtr>         ChannelOutputSList;

  class SegmentationRepresentation;
  typedef boost::shared_ptr<SegmentationRepresentation> SegmentationRepresentationSPtr;

  class EspinaCore_EXPORT SegmentationOutput
  : public FilterOutput
  {
  public:
    explicit SegmentationOutput(Filter *filter = 0, const FilterOutputId &id = INVALID_OUTPUT_ID);

    bool dumpSnapshot (const QString &prefix, Snapshot &snapshot, bool saveEditedRegions);

    virtual bool isValid() const;

    virtual EspinaRegion region() const;

    bool isEdited() const;

    void push(EditedRegionSList editedRegions);

    /// clear output's edited regions
    void clearEditedRegions();

    /// dump output's edited regions information to snapshot
    void dumpEditedRegions(const QString &prefix, Snapshot &snapshot);

    EditedRegionSList editedRegions() const;

    /// restore output's edited regions information from cache
    void restoreEditedRegions(const QDir &cacheDir, const QString &ouptutId);

    /// replace current edited regions
    void setEditedRegions(EditedRegionSList regions);

    void setRepresentation(const OutputRepresentationName &name, SegmentationRepresentationSPtr representation);

    SegmentationRepresentationSPtr representation(const OutputRepresentationName &name) const
    { return m_representations.value(name, SegmentationRepresentationSPtr()); }

  private:
    
    QMap<OutputRepresentationName, SegmentationRepresentationSPtr> m_representations;

    EditedRegionSList m_editerRegions;
  };

  typedef SegmentationOutput                  * SegmentationOutputPtr;
  typedef boost::shared_ptr<SegmentationOutput> SegmentationOutputSPtr;
  typedef QList<SegmentationOutputSPtr>         SegmentationOutputSList;
} // namespace EspINA

#endif // OUTPUT_H
