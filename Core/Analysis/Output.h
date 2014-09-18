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

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include "Core/Analysis/Data.h"
#include <Core/Utils/NmVector3.h>

// Qt
#include <QMap>
#include <QXmlStreamWriter>

class QDir;

namespace ESPINA
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
 		/* \brief Output class constructor.
 		 * \param[in] filter, filter object smart pointer.
 		 * \param[in] if, Output::Id specifier.
 		 *
 		 */
    explicit Output(FilterPtr filter, const Output::Id& id);

 		/* \brief Output class destructor.
 		 *
 		 */
    virtual ~Output();

 		/* \brief Returns the filter owner of this output.
 		 *
 		 */
    FilterPtr filter() const
    { return m_filter; }

 		/* \brief Returns this output's id.
 		 *
 		 */
    Id id() const
    { return m_id; }

 		/* \brief Sets the spacing.
 		 * \param[in] spacing.
 		 *
 		 */
    void setSpacing(const NmVector3& spacing);

 		/* \brief Returns the spacing.
 		 *
 		 */
    NmVector3 spacing() const;

 		/* \brief Returns a snapshot data for this output.
 		 * \param[in] storage, temporal storage object where data files will be saved.
 		 * \param[out] xml, information of the output data in xml format.
 		 * \param[in] prefix, prefix for the data files.
 		 *
 		 */
    Snapshot snapshot(TemporalStorageSPtr storage,
                      QXmlStreamWriter       &xml,
                      const QString          &prefix) const;

 		/* \brief Returns true if this output is valid.
 		 *
 		 */
    bool isValid() const;

 		/* \brief Returns true if this output has been modified.
 		 *
 		 */
    bool isEdited() const;

 		/* \brief Clears the mofications made to this output.
 		 *
 		 */
    void clearEditedRegions();

 		/* \brief Sets a data object for this output.
 		 * \param[in] data, data object smart pointer.
 		 *
 		 */
    void setData(DataSPtr data);

 		/* \brief Removes a data object from this output.
 		 *
 		 */
    void removeData(const Data::Type& type);

 		/* \brief Returns the data of the specified type.
 		 * \param[in] type, data type.
 		 *
 		 */
    DataSPtr data(const Data::Type& type) const;

 		/* \brief Returns true if the output has a data of the specified type.
 		 * \param[in] type, data type.
 		 *
 		 */
    bool hasData(const Data::Type& type) const;

    /** \brief Request necessary pipeline execution to update this output.
     *
     */
    void update();

 		/* \brief Returns true if the output need to save data to disk.
 		 *
 		 */
    bool hasToBeSaved() const;

 		/* \brief Returns the bounds of the output.
 		 *
 		 * TODO: Representation may have different bounds, in which case,
     * this function will be needed to represent the bounding box of all those regions
 		 *
 		 */
    virtual Bounds bounds() const;

 		/* \brief Returns the time stamp of the last modification.
 		 *
 		 */
    TimeStamp lastModified()
    { return m_timeStamp; }

 		/* \brief Increments modification time for the output.
 		 *
 		 */
    void updateModificationTime()
    { m_timeStamp = s_tick++; }

  protected slots:
		/* \brief Emits modification signal for this object.
		 *
		 */
    void onDataChanged();

  signals:
    void modified();

  private:
    static TimeStamp s_tick;
    static const int INVALID_OUTPUT_ID;

    FilterPtr m_filter;
    Id        m_id;
    NmVector3 m_spacing;

    TimeStamp m_timeStamp;

    EditedRegionSList m_editedRegions;

    QMap<Data::Type, DataProxySPtr> m_data;
  };

  using OutputIdList = QList<Output::Id>;
} // namespace ESPINA

#endif // ESPINA_OUTPUT_H
