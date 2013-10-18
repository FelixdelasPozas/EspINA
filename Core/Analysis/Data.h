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
#include <QDir>

namespace EspINA
{
  class Data;
  using DataSPtr  = std::shared_ptr<Data>;
  using DataSList = QList<DataSPtr>;

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

    void setOutput(OutputPtr output);

    /** \brief Last modification time stamp
     */
    TimeStamp lastModified()
    { return m_timeStamp; }

    virtual bool dumpSnapshot (const QString &prefix, Snapshot &snapshot) const = 0;

    virtual bool isValid() const = 0;

    virtual Bounds bounds() = 0;

    virtual bool setInternalData(DataSPtr rhs) = 0;

    virtual void addEditedRegion(const Bounds& region, int cacheId = -1) = 0;

    /// Whether output has been manually edited
    virtual bool isEdited() const = 0;

    virtual void clearEditedRegions() = 0;

    /// Update output's edited region list
    virtual void commitEditedRegions(bool withData) const = 0;

    virtual void restoreEditedRegions(const QDir &cacheDir, const QString &outputId) = 0;

  signals:
    void dataChanged();//former representationChanged

  protected:
    explicit Data(OutputPtr output)
    : m_output(output), m_timeStamp(s_tick++) {}

    void updateModificationTime() 
    {
      m_timeStamp = s_tick++;
    }

  protected:
    OutputPtr m_output;

  private:
    TimeStamp m_timeStamp;
  };

} // namespace EspINA

#endif // ESPINA_DATA_H