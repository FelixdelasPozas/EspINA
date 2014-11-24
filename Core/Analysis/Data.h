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

#ifndef ESPINA_DATA_H
#define ESPINA_DATA_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include <Core/Utils/Bounds.h>
#include "Persistent.h"

#include <QMutex>

namespace ESPINA
{
  class ChangeSignalDelayer;

  class Data;
  using DataSPtr  = std::shared_ptr<Data>;
  using DataSList = QList<DataSPtr>;

  class DataProxy;
  using DataProxySPtr = std::shared_ptr<DataProxy>;

  class EspinaCore_EXPORT Data
  : public QObject
  {
    Q_OBJECT

    static TimeStamp s_tick;

  public:
    using Type = QString;

  public:
    /** \brief Data class constructor.
     *
     */
    virtual ~Data()
    {}

    /** \brief Returns the type of data.
     *
     */
    virtual Data::Type type() const = 0;

    /** \brief Creates a proxy for the data type.
     *
     */
    virtual DataSPtr createProxy() const = 0;

    /** \brief Sets the data output.
     * \param[in] output Output object smart pointer.
     *
     */
    void setOutput(OutputPtr output)
    { m_output = output; }

    /** \brief Returns the list of data types on which this
     *         data type relies on
     *
     */
    QList<Data::Type> dependencies() const;

    /** \brief Returns the time stamp of the last modification to the data.
     *
     */
    virtual TimeStamp lastModified()
    { return m_timeStamp; }

    /** \brief Returns the list of bounds of the edited regions of the data.
     *
     */
    virtual BoundsList editedRegions() const
    { return m_editedRegions; }

    /** \brief Set current data edited regions
     *
     */
    virtual void setEditedRegions(const BoundsList &regions)
    { m_editedRegions = regions; }

    /** \brief Set context to look for data on fetch request
     * \param[in] storage temporal storage where data snasphots can be loaded from.
     * \param[in] path storage path where data snapshosts will be loaded from
     * \param[in] id identifier of stored data snapshosts
     *
     */
    void setFetchContext(const TemporalStorageSPtr storage, const QString &path, const QString &id);

    /** \brief Recover data from Persistent Storage.
     */
    bool fetchData();

    /** \brief Clears the edited regions list.
     *
     */
    virtual void clearEditedRegions()
    { m_editedRegions.clear(); }

    /** \brief Return the byte arrays needed to save this object between sessions.
     * \param[in] storage temporal storage where data snasphots can be loaded from
     * \param[in] path storage path where data snapshosts will be saved to
     * \param[in] id identifier to store data snapshosts
     *
     *  Temporal storage may be also used to store temporal files where snapshot generation
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const = 0;

    /** \brief Returns a snapshot object of the edited regions of the data.
     * \param[in] storage temporal storage where edited regions snasphots can be loaded from
     * \param[in] path storage path where edited regions snapshosts will be saved to
     * \param[in] id identifier to store edited regions snapshosts
     *
     *  Temporal storage may be also used to store temporal files where snapshot generation
     */
    virtual Snapshot editedRegionsSnapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const = 0;

    /** \brief Restore data edited regions from its snapshots.
     * \param[in] storage temporal storage where edited regions snasphots can be loaded from
     * \param[in] path storage path where edited regions snapshosts will be loaded from
     * \param[in] id identifier to store edited regions snapshosts
     *
     * PRE: Previously edited regions bounds have been restored
     */
    virtual void restoreEditedRegions(TemporalStorageSPtr storage, const QString &path, const QString &id) = 0;

    /** \brief Returns true if the object has been correctly initialized and contains data.
     *
     */
    virtual bool isValid() const = 0;

    /** \brief Returns true if the object is empty.
     *
     */
    virtual bool isEmpty() const = 0;

    /** \brief Returns the bounds of the contained data.
     *
     */
    virtual Bounds bounds() const = 0;

    /** \brief Sets the spacing of the data.
     *
     */
    virtual void setSpacing(const NmVector3& spacing) = 0;

    /** \brief Returns the spacing of the data.
     *
     */
    virtual NmVector3 spacing() const = 0;

    /** \brief Returns true if the object has been edited.
     *
     */
    bool isEdited() const
    { return !editedRegions().isEmpty(); }

    /** \brief Undo last edition operation.
     *
     */
    virtual void undo() = 0;

    /** \brief Return memory usage in bytes.
     *
     * Returns the amount of memory allocated by the object
     */
    virtual size_t memoryUsage() const = 0;

  signals:
    void dataChanged(); //former representationChanged

  protected:
    /** \brief Data class constructor.
     *
     */
    explicit Data()
    : m_output   {nullptr}
    , m_timeStamp{s_tick++}
    , m_mutex(QMutex::Recursive)
    {
    }

    /** \brief Increments the modification time and signals the modification of the data.
     *
     */
    void updateModificationTime()
    {
      m_timeStamp = s_tick++;
      emit dataChanged();
    }

    void addEditedRegion(const Bounds &bounds)
    { m_editedRegions << bounds; }

    virtual bool fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id) = 0;

  private:
    /** \brief Returns the list of data types on which this
     *         data type relies on
     *
     */
    virtual QList<Data::Type> updateDependencies() const = 0;

  protected:
    OutputPtr  m_output;

    QString             m_path;
    QString             m_id;
    TemporalStorageSPtr m_storage;

  private:
    TimeStamp  m_timeStamp;
    BoundsList m_editedRegions;

    QMutex m_mutex;

    friend class Output;
    friend class ChangeSignalDelayer;
  };

  enum class DataUpdatePolicy { Request, Ignore};
} // namespace ESPINA

#endif // ESPINA_DATA_H
