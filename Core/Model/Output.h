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

  class IEspinaRepresentation;
  typedef boost::shared_ptr<IEspinaRepresentation> EspinaRepresentationSPtr;
  typedef QList<EspinaRepresentationSPtr>          EspinaRepresentationList;

  typedef int FilterOutputId;

  class FilterOutput
  : public QObject
  {
    static const int INVALID_OUTPUT_ID;

    Q_OBJECT
  public:
    class OutputType;
    typedef boost::shared_ptr<OutputType> OutputTypeSPtr;
    typedef QList<OutputTypeSPtr>         OutputTypeList;
    typedef QString                       OutputTypeName;

    typedef QPair<QString, EspinaRegion> NamedRegion;
    typedef QList<NamedRegion>           NamedRegionList;

  public:
    explicit FilterOutput(Filter               *filter = NULL,
                          const FilterOutputId &id     = INVALID_OUTPUT_ID);

    bool dumpSnapshot (const QString &prefix, Snapshot &snapshot);
    bool fetchSnapshot(const QString &prefix);

    bool isValid() const;

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

    void setData(const OutputTypeName &name, OutputTypeSPtr data)
    { m_data[name] = data; }

    OutputTypeSPtr data(const OutputTypeName &name) const
    { return m_data.value(name, OutputTypeSPtr()); }

    void addRepresentation(EspinaRepresentationSPtr prototype)
    { m_repPrototypes << prototype; }

    EspinaRepresentationList representations() const
    { return m_repPrototypes; }

  signals:
    void modified();

  private:
    FilterOutputId m_id;
    bool           m_isCached; /// Whether output is used by a segmentation
    FilterPtr      m_filter;
    EspinaRegion   m_region;
    EspinaRepresentationList m_repPrototypes;

    QMap<OutputTypeName, OutputTypeSPtr> m_data;
  };

  typedef boost::shared_ptr<FilterOutput> OutputSPtr;
  typedef QList<OutputSPtr> OutputList;

} // namespace EspINA

#endif // OUTPUT_H
