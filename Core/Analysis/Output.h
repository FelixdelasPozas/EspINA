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


#ifndef ESPINA_OUTPUT_H
#define ESPINA_OUTPUT_H

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Analysis/Data.h"

#include <QMap>

class QDir;
namespace EspINA
{

  class EspinaCore_EXPORT Output
  : public QObject
  {
    Q_OBJECT
  public:
    using DataTypeList = QList<Data::Type>;
    using Id = unsigned int;

    class EditedRegion;

    using EditedRegionSPtr  = std::shared_ptr<EditedRegion>;
    using EditedRegionSList = QList<EditedRegionSPtr>;

    using DataSPtr  = std::shared_ptr<Data>;
    using DataSList = QList<DataSPtr>;

  public:
    explicit Output(FilterPtr filter, const Output::Id& id);

    FilterPtr filter() const
    { return m_filter; }

    Id id() const
    { return m_id; }

    bool isValid() const;

    bool isEdited() const;

    void push(EditedRegionSList editedRegions);

    /// clear output's edited regions
    void clearEditedRegions();

    /// dump output's edited regions information to snapshot
    void dumpEditedRegions(const QString &prefix, Snapshot &snapshot);

//     EditedRegionSList editedRegions() const;

    /// restore output's edited regions information from cache
    void restoreEditedRegions(const QDir &cacheDir, const QString &ouptutId);

//     /// replace current edited regions
//     void setEditedRegions(EditedRegionSList regions);

    void setData(const Data::Type& type, DataSPtr data);

    DataSPtr data(const Data::Type& type) const
    { return m_data.value(type, DataSPtr()); }

    /** \brief Request necessary pipeline execution to update this output
     *
     */
    void update();

    bool hasToBeSaved() const//isCached() const
    { return m_hasToBeSaved; }

    void markToSave(bool value)
    { m_hasToBeSaved = value; }

    bool dumpSnapshot (const QString &prefix, Snapshot &snapshot, bool saveEditedRegions);

    // TODO: Representation may have different bounds, in which case,
    // this function will be needed to represent the bounding box of all those regions
    virtual Bounds bounds() const;

    TimeStamp lastModified()
    { return m_timeStamp; }

    // NOTE: Temporal solution to change output when filters are updated
    void updateModificationTime() 
    { m_timeStamp = s_tick++; }

  protected slots:
    void onDataChanged(); // former onRepresentationChange

  signals:
    void modified();

  private:
    static TimeStamp s_tick;
    static const int INVALID_OUTPUT_ID;

    FilterPtr m_filter;
    Id        m_id;

    TimeStamp m_timeStamp;

    bool              m_hasToBeSaved;
    EditedRegionSList m_editedRegions;

    QMap<Data::Type, DataSPtr> m_data;
  };

  class Output::EditedRegion
  {
  public:
    EditedRegion(int id, const Data::Type& name, const Bounds& region)
    : Id(id), Name(name), Region(region){}
    virtual ~EditedRegion() {}

    int Id;
    Data::Type Name;
    Bounds     Region;

    virtual bool dump(QDir           cacheDir,
                      const QString &regionName,
                      Snapshot      &snapshot) const = 0;
  };

  using OutputIdList = QList<Output::Id>;
} // namespace EspINA

#endif // ESPINA_OUTPUT_H