/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ESPINA_PERSISTENT_H
#define ESPINA_PERSISTENT_H

#include <memory>
#include <QString>
#include <QDebug>
#include <Core/Utils/TemporalStorage.h>

namespace EspINA {

  using State = QString;

  class Persistent
  {
  public:
    using Uuid = QUuid;

  public:
    explicit Persistent() 
    : m_quuid{QUuid::createUuid()}
    {}
    virtual ~Persistent() {}

    Uuid uuid() const
    { return m_quuid; }

    void setUuid(Uuid id)
    { m_quuid = id; }

    void setStorage(TemporalStorageSPtr storage)
    { m_storage = storage; }

    TemporalStorageSPtr storage() const
    { return m_storage; }

    void setName(const QString& name)
    { m_name = name; }

    QString name() const
    { return m_name; }


    virtual void restoreState(const State& state) = 0;

    //NOTE: If we want to allow ReadOnly objects to keep state we need to avoid new lines
    virtual State state() const = 0;

    virtual Snapshot snapshot() const = 0;

    /** \brief Release all resources loaded in memory
     *
     */
    virtual void unload() = 0;

  private:
    Uuid                m_quuid;
    QString             m_name;
    TemporalStorageSPtr m_storage;
  };

  using PersistentPtr = Persistent *;
}

#endif // ESPINA_PERSISTENT_H
