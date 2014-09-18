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
    /* \brief Data class constructor.
     *
     */
    virtual ~Data()
    {}

    /* \brief Returns the type of data.
     *
     */
    virtual Data::Type type() const = 0;

    /* \brief Creates a proxy for the data type.
     *
     */
    virtual DataProxySPtr createProxy() const = 0;

    /* \brief Sets the data output.
     * \param[in] output, Output object smart pointer.
     *
     */
    void setOutput(OutputPtr output)
    { m_output = output; }

    /** \brief Returns the time stamp of the last modification to the data.
     *
     */
    virtual TimeStamp lastModified()
    { return m_timeStamp; }

    /* \brief Returns the list of bounds of the edited regions of the data.
     *
     */
    virtual BoundsList editedRegions() const
    { return m_editedRegions; }

    /* \brief Clears the edited regions list.
     *
     */
    virtual void clearEditedRegions()
    { m_editedRegions.clear(); }

    /** \brief Recover output data from Persistent Storage.
     * \param[in] storage, smart pointer of the temporal storage where to retrieve the data.
     * \param[in] prefix, prefix of the filenames.
     *
     */
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString &prefix) = 0;

    /** \brief Return the byte arrays needed to save this object between sessions.
     * \param[in] storage, smart pointer of the temporal storage where to save the data.
     * \param[in] prefix, prefix of the filenames.
     *
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &prefix) const = 0;

    /* \brief Returns a snapshot object of the edited regions of the data.
     *
     */
    virtual Snapshot editedRegionsSnapshot() const = 0;

    /* \brief Returns true if the object has been correctly initialized and contains data.
     *
     */
    virtual bool isValid() const = 0;

    /* \brief Returns true if the object is empty.
     *
     */
    virtual bool isEmpty() const = 0;

    /* \brief Returns the bounds of the contained data.
     *
     */
    virtual Bounds bounds() const = 0;

    /* \brief Sets the spacing of the data.
     *
     */
    virtual void setSpacing(const NmVector3& spacing) = 0;

    /* \brief Returns the spacing of the data.
     *
     */
    virtual NmVector3 spacing() const = 0;

    /* \brief Returns true if the object has been edited.
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
    void dataChanged();//former representationChanged

  protected:
    /* \brief Data class constructor.
     *
     */
    explicit Data()
    : m_output   {nullptr}
    , m_timeStamp{s_tick++}
    {}

    /* \brief Increments the modification time and signals the modification of the data.
     *
     */
    void updateModificationTime()
    {
      m_timeStamp = s_tick++;
      emit dataChanged();
    }

  protected:
    OutputPtr  m_output;
    BoundsList m_editedRegions;

  private:
    TimeStamp m_timeStamp;

    friend class Output;
    friend class ChangeSignalDelayer;
  };

} // namespace ESPINA

#endif // ESPINA_DATA_H
