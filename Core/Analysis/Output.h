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
#include <Core/Utils/NmVector3.h>

#include <QMap>
#include <QXmlStreamWriter>

class QDir;
namespace EspINA
{

  class EspinaCore_EXPORT Output
  : public QObject
  {
    Q_OBJECT
  public:
    using DataTypeList = QList<Data::Type>;
    using Id = int;

    class EditedRegion;

    using EditedRegionSPtr  = std::shared_ptr<EditedRegion>;
    using EditedRegionSList = QList<EditedRegionSPtr>;

    using DataSPtr  = std::shared_ptr<Data>;
    using DataSList = QList<DataSPtr>;

  public:
    explicit Output(FilterPtr filter, const Output::Id& id);
    virtual ~Output();

    FilterPtr filter() const
    { return m_filter; }

    Id id() const
    { return m_id; }

    void setSpacing(const NmVector3& spacing);

    NmVector3 spacing() const;

    Snapshot snapshot(TemporalStorageSPtr storage,
                      QXmlStreamWriter       &xml,
                      const QString          &prefix) const;

    bool isValid() const;

    bool isEdited() const;

    void clearEditedRegions();

//     /// restore output's edited regions information from cache
//     void restoreEditedRegions(const QDir &cacheDir, const QString &ouptutId);

    void setData(DataSPtr data);

    void removeData(const Data::Type& type);

    DataSPtr data(const Data::Type& type) const;

    bool hasData(const Data::Type& type) const;

    /** \brief Request necessary pipeline execution to update this output
     *
     */
    void update();

    bool hasToBeSaved() const;

//     void markToSave(bool value)
//     { m_hasToBeSaved = value; }

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
    NmVector3 m_spacing;

    TimeStamp m_timeStamp;

//     bool              m_hasToBeSaved;
    EditedRegionSList m_editedRegions;

    QMap<Data::Type, DataProxySPtr> m_data;
  };

  using OutputIdList = QList<Output::Id>;
} // namespace EspINA

#endif // ESPINA_OUTPUT_H
