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

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include <Core/Utils/Bounds.h>
#include "Persistent.h"

namespace EspINA
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
    virtual ~Data(){}

    virtual Data::Type type() const = 0; 

    virtual DataProxySPtr createProxy() const = 0;

    void setOutput(OutputPtr output)
    { m_output = output; }

    /** \brief Last modification time stamp
     */
    virtual TimeStamp lastModified()
    { return m_timeStamp; }

    virtual BoundsList editedRegions() const
    { return m_editedRegions; }

    virtual void clearEditedRegions()
    { m_editedRegions.clear(); }

    /** \brief Recover output data from Persistent Storage
     *
     */
    virtual bool fetchData(const TemporalStorageSPtr storage, const QString &prefix) = 0;

    /** \brief Return the byte arrays needed to save this object between sessions
     *
     */
    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString &prefix) const = 0;

    virtual Snapshot editedRegionsSnapshot() const = 0;

    virtual bool isValid() const = 0;

    virtual Bounds bounds() const = 0;

    virtual void setSpacing(const NmVector3& spacing) = 0;

    virtual NmVector3 spacing() const = 0;

    bool isEdited() const
    { return !editedRegions().isEmpty(); }

    /** \brief Undo last edition operation
     *
     */
    virtual void undo() = 0;

    /** \brief Return memory usage in bytes
     *
     * Returns the amount of memory allocated by the object
     */
    virtual size_t memoryUsage() const = 0;

  signals:
    void dataChanged();//former representationChanged

  protected:
    explicit Data()
    : m_output(nullptr), m_timeStamp(s_tick++) {}

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

} // namespace EspINA

#endif // ESPINA_DATA_H
