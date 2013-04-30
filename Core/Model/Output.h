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

#include <Core/EspinaRegion.h>

#include <boost/shared_ptr.hpp>
#include <QMap>

namespace EspINA
{

  class Filter;

  class GraphicalRepresentation;
  typedef boost::shared_ptr<GraphicalRepresentation> GraphicalRepresentationSPtr;
  typedef QList<GraphicalRepresentationSPtr>          EspinaRepresentationList;

 typedef int FilterOutputId;

  class FilterOutput
  : public QObject
  {
    Q_OBJECT
  protected:
    static const int INVALID_OUTPUT_ID;

  public:
    class OutputRepresentation;
    typedef boost::shared_ptr<OutputRepresentation> OutputRepresentationSPtr;
    typedef QList<OutputRepresentationSPtr>         OutputRepresentationSList;
    typedef QString                                 OutputRepresentationName;

    typedef QPair<QString, EspinaRegion> NamedRegion;
    typedef QList<NamedRegion>           NamedRegionList;

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

    void setRegion(const EspinaRegion &region);

    EspinaRegion region() const
    { return m_region; }

    void addRepresentation(GraphicalRepresentationSPtr prototype)
    { m_repPrototypes << prototype; }

    EspinaRepresentationList representations() const
    { return m_repPrototypes; }

  signals:
    void modified();

  protected:
    FilterOutputId m_id;
    bool           m_isCached; /// Whether output is used by a segmentation
    FilterPtr      m_filter;
    EspinaRegion   m_region;
    EspinaRepresentationList m_repPrototypes;
  };

  typedef FilterOutput                  * OutputPtr;
  typedef boost::shared_ptr<FilterOutput> OutputSPtr;
  typedef QList<OutputSPtr> OutputSList;

  class ChannelRepresentation;
  typedef boost::shared_ptr<ChannelRepresentation> ChannelRepresentationSPtr;

  class ChannelOutput
  : public FilterOutput
  {
  public:
    explicit ChannelOutput(Filter *filter = 0, const FilterOutputId &id = INVALID_OUTPUT_ID);

    virtual bool isValid() const;

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

  class SegmentationOutput
  : public FilterOutput
  {
  public:
    explicit SegmentationOutput(Filter *filter = 0, const FilterOutputId &id = INVALID_OUTPUT_ID);

    bool dumpSnapshot (const QString &prefix, Snapshot &snapshot);
    bool fetchSnapshot(const QString &prefix);

    virtual bool isValid() const;

    bool isEdited() const;

    /// clear output's edited regions
    void clearEditedRegions();

    /// dump output's edited regions information to snapshot
    void dumpEditedRegions(const QString &prefix, Snapshot &snapshot);

    NamedRegionList editedRegions() const;

    /// restore output's edited regions information from cache
    void restoreEditedRegions(const QString &prefix);

    /// replace current edited regions
    void setEditedRegions(const NamedRegionList &regions);

    void setRepresentation(const OutputRepresentationName &name, SegmentationRepresentationSPtr representation)
    { m_representations[name] = representation; }

    SegmentationRepresentationSPtr representation(const OutputRepresentationName &name) const
    { return m_representations.value(name, SegmentationRepresentationSPtr()); }

  private:
    QMap<OutputRepresentationName, SegmentationRepresentationSPtr> m_representations;
  };

  typedef SegmentationOutput                  * SegmentationOutputPtr;
  typedef boost::shared_ptr<SegmentationOutput> SegmentationOutputSPtr;
  typedef QList<SegmentationOutputSPtr>         SegmentationOutputSList;
} // namespace EspINA

#endif // OUTPUT_H
