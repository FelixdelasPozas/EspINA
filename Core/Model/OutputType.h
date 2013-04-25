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


#ifndef OUTPUTTYPE_H
#define OUTPUTTYPE_H

#include "Core/Model/Output.h"

namespace EspINA
{
  typedef unsigned long long EspinaTimeStamp;

  class FilterOutput::OutputType
  : public QObject
  {
    static EspinaTimeStamp s_tick;

  public:
    virtual ~OutputType() {}

    virtual FilterOutput::OutputTypeName type() const = 0; 

    void setOutput(FilterOutput *output)
    { m_output = output; }

    virtual bool setInternalData(OutputTypeSPtr rhs) = 0;

    EspinaTimeStamp timeStamp()
    { return m_timeStamp; }

    virtual bool dumpSnapshot (const QString &prefix, Snapshot &snapshot) = 0;
    virtual bool fetchSnapshot(const QString &prefix) = 0;

    /// Whether output has been manually edited
    virtual bool isEdited() const = 0;

    virtual bool isValid() const = 0;

    virtual FilterOutput::NamedRegionList editedRegions() const = 0;

    virtual void clearEditedRegions() = 0;

    virtual void dumpEditedRegions(const QString &prefix) const = 0;
    virtual void restoreEditedRegion(const EspinaRegion &region, const QString &prefix) = 0;

  protected:
    explicit OutputType(FilterOutput *output)
    : m_output(output), m_timeStamp(s_tick++) {}

    void markAsModified() 
    {
      m_timeStamp = s_tick++;
    }

  protected:
    FilterOutput   *m_output;

  private:
    EspinaTimeStamp m_timeStamp;
  };

  class MeshOutputType
  : public FilterOutput::OutputType
  {

  };

} // namespace EspINA

#endif // OUTPUTTYPE_H
