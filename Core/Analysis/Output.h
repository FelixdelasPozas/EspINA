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

#include <Core/Utils/Vector3.hxx>
#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include "Core/Analysis/Data.h"
#include "DataProxy.h"
#include <QMap>
#include <QXmlStreamWriter>

class QDir;

namespace ESPINA
{

  struct Invalid_Output_Expection{};
  struct Unavailable_Output_Data_Exception {};

  class EspinaCore_EXPORT Output
  : public QObject
  {
    Q_OBJECT
  private:
    class Locker
    {
    public:
      explicit Locker(QReadWriteLock &locker)
      : m_locker(locker)
      {}

      virtual ~Locker()
      { m_locker.unlock();}

    protected:
      QReadWriteLock &m_locker;
    };

    class ReadLocker
    : public Locker
    {
    public:
      explicit ReadLocker(QReadWriteLock &locker)
      : Locker(locker)
      { m_locker.lockForRead(); }
    };

    class WriteLocker
    : public Locker
    {
    public:
      explicit WriteLocker(QReadWriteLock &locker)
      : Locker(locker)
      { m_locker.lockForWrite(); }
    };

  public:
    using DataTypeList = QList<Data::Type>;
    using Id = int;

    class EditedRegion;

    using EditedRegionSPtr  = std::shared_ptr<EditedRegion>;
    using EditedRegionSList = QList<EditedRegionSPtr>;

    using DataSPtr      = std::shared_ptr<Data>;
    using ConstDataSPtr = std::shared_ptr<const Data>;
    using DataSList     = QList<DataSPtr>;

    template<typename T>
    class ReadLockData
    {
    public:
      explicit ReadLockData()
      {}

      explicit ReadLockData(std::shared_ptr<T> data)
      : m_dataProxy(data)
      , m_locker{new ReadLocker(dynamic_cast<DataProxy *>(data.get())->m_lock)}
      {}

      bool isNull() const
      { return !m_dataProxy || !m_locker; }

      operator std::shared_ptr<const T>() const
      { return std::const_pointer_cast<const T>(m_dataProxy); }

      const T * operator -> () const
      { return m_dataProxy.get(); }

    protected:
      explicit ReadLockData(std::shared_ptr<T> data, Locker *locker)
      : m_dataProxy(data)
      , m_locker(locker)
      {}

    protected:
      std::shared_ptr<T> m_dataProxy;

    private:
      std::unique_ptr<Locker> m_locker;
    };

    template<typename T>
    class WriteLockData
    : public ReadLockData<T>
    {
    public:
      explicit WriteLockData()
      {}

      explicit WriteLockData(std::shared_ptr<T> data)
      : ReadLockData<T>(data, new WriteLocker(dynamic_cast<DataProxy *>(data.get())->m_lock))
      {
      }

      operator std::shared_ptr<T>()
      { return this->m_dataProxy; }

       operator DataSPtr()
      { return this->m_dataProxy; }

      T * operator ->()
      { return this->m_dataProxy.get(); }
    };

  public:
    /** \brief Output class constructor.
     * \param[in] filter filter object smart pointer.
     * \param[in] id Output::Id specifier.
     *
     */
    explicit Output(FilterPtr filter, const Output::Id& id, const NmVector3 &spacing);

    /** \brief Output class destructor.
     *
     */
    virtual ~Output() override;

    /** \brief Returns the filter owner of this output.
     *
     */
    FilterPtr filter() const
    { return m_filter; }

    /** \brief Returns this output's id.
     *
     */
    Id id() const
    { return m_id; }

    /** \brief Sets the spacing.
     * \param[in] spacing
     *
     */
    void setSpacing(const NmVector3& spacing);

    /** \brief Returns the spacing.
     *
     */
    NmVector3 spacing() const;

    /** \brief Returns a snapshot data for this output.
     * \param[in]  storage temporal storage object where data files will be saved.
     * \param[out] xml     information of the output data in xml format.
     * \param[in]  path    data snapshots path
     *
     */
    Snapshot snapshot(TemporalStorageSPtr storage,
                      QXmlStreamWriter       &xml,
                      const QString          &path) const;

    /** \brief Returns true if this output is valid.
    *
    */
    bool isValid() const;

    /** \brief Returns true if this output has been modified.
     *
     */
    bool isEdited() const;

    /** \brief Clears the mofications made to this output.
     *
     */
    void clearEditedRegions();

    /** \brief Sets a data object for this output.
     * \param[in] data data object smart pointer.
     *
     */
    void setData(DataSPtr data);

    /** \brief Removes a data object from this output.
     *
     */
    void removeData(const Data::Type& type);

    /** \brief Returns the data of the specified type.
     * \param[in] type data type.
     *
     */
    template<typename T>
    ReadLockData<T> readLockData(const Data::Type &type) const throw (Unavailable_Output_Data_Exception)
    {  return ReadLockData<T>(data<T>(type)); }

    /** \brief Returns the data of the specified type.
     * \param[in] type data type.
     *
     */
    template<typename T>
    WriteLockData<T> writeLockData(const Data::Type &type) throw (Unavailable_Output_Data_Exception)
    { return WriteLockData<T>(data<T>(type)); }

    /** \brief Returns true if the output has a data of the specified type.
     * \param[in] type data type.
     *
     */
    bool hasData(const Data::Type& type) const;

    /** \brief Returns the number of valid Data in the output object.
     *
     */
    unsigned int numberOfDatas() const;

    /** \brief Request necessary pipeline execution to update output data
     *
     */
    void update();

    /** \brief Request necessary pipeline execution to update output data of given type
     *
     */
    void update(const Data::Type &type);

    /** \brief Returns the bounds of the output.
     *
     * NOTE: Representation may have different bounds, in which case,
     * this function will be needed to represent the bounding box of all those regions
     *
     */
    Bounds bounds() const;

    /** \brief Returns the time stamp of the last modification.
     *
     */
    TimeStamp lastModified() const
    { return m_timeStamp; }

    /** \brief Increments modification time for the output.
     *
     */
    void updateModificationTime()
    { m_timeStamp = s_tick++; }

  private slots:
    /** \brief Emits modification signal for this object.
     *
     */
    void onDataChanged();

    /** \brief Returns true if the output need to save data to disk.
     *
     */
    bool isSegmentationOutput() const;

  signals:
    void modified();

  private:
    template<typename T>
    std::shared_ptr<T> data(const Data::Type &type) const throw (Unavailable_Output_Data_Exception)
    {
      if (!m_data.contains(type)) throw Unavailable_Output_Data_Exception();

      return std::dynamic_pointer_cast<T>(m_data.value(type));
    }

    DataProxy *proxy(const Data::Type &type);

  private:
    static TimeStamp s_tick;
    static const int INVALID_OUTPUT_ID;

    QMutex    m_mutex;
    FilterPtr m_filter;
    Id        m_id;
    NmVector3 m_spacing;

    TimeStamp m_timeStamp;

    EditedRegionSList m_editedRegions;

    QMap<Data::Type, DataSPtr>       m_data;
  };

  using OutputIdList = QList<Output::Id>;

  template <typename T>
  Output::ReadLockData<T> outputReadLockData(Output *output, DataUpdatePolicy policy)
  {
    auto type = T::TYPE;

    if (policy == DataUpdatePolicy::Request)
    {
      output->update(type);
    }

    return output->readLockData<T>(type);
  }

  template <typename T>
  Output::WriteLockData<T> outputWriteLockData(Output *output, DataUpdatePolicy policy)
  {
    auto type = T::TYPE;

    if (policy == DataUpdatePolicy::Request)
    {
      output->update(type);
    }

    return output->writeLockData<T>(type);
  }

} // namespace ESPINA

#endif // ESPINA_OUTPUT_H
