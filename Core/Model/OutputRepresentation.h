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


#ifndef OUTPUTREPRESENTATION_H
#define OUTPUTREPRESENTATION_H

#include "EspinaCore_Export.h"

#include "Core/Model/Output.h"

#include <itkImageRegionIteratorWithIndex.h>

class QDir;
class vtkAlgorithmOutput;
class vtkDiscreteMarchingCubes;
class vtkImageConstantPad;
class vtkImplicitFunction;
class vtkPolyData;

namespace EspINA
{
  typedef itk::ImageRegionIteratorWithIndex<itkVolumeType>      itkVolumeIterator;
  typedef itk::ImageRegionConstIteratorWithIndex<itkVolumeType> itkVolumeConstIterator;

  typedef unsigned long long EspinaTimeStamp;

  class EspinaCore_EXPORT FilterOutput::OutputRepresentation
  : public QObject
  {
    static EspinaTimeStamp s_tick;
    Q_OBJECT
  public:
  virtual ~OutputRepresentation(){}

    virtual FilterOutput::OutputRepresentationName type() const = 0; 

    void setOutput(FilterOutput *output);

    EspinaTimeStamp timeStamp()
    { return m_timeStamp; }

    virtual bool dumpSnapshot (const QString &prefix, Snapshot &snapshot) const = 0;

    virtual bool isValid() const = 0;

    // TODO: Use this name to avoid collisions with bounds methods already defined
    // in some representations. These methods will be deprecated in next version
    // in favour of using EspinaBounds (former EspinaRegion)
    virtual EspinaRegion representationBounds() = 0;

  signals:
    void representationChanged();

  protected:
    explicit OutputRepresentation(FilterOutput *output)
    : m_output(output), m_timeStamp(s_tick++) {}

    void updateModificationTime() 
    {
      m_timeStamp = s_tick++;
    }

  protected:
    FilterOutput   *m_output;

  private:
    EspinaTimeStamp m_timeStamp;
  };

  class EspinaCore_EXPORT ChannelRepresentation
  : public FilterOutput::OutputRepresentation
  {
  public:
    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
    { return false; }

    virtual bool setInternalData(ChannelRepresentationSPtr rhs) = 0;

  protected:
    explicit ChannelRepresentation(FilterOutput *output)
    : FilterOutput::OutputRepresentation(output) {}
  };

  class EspinaCore_EXPORT SegmentationRepresentation
  : public FilterOutput::OutputRepresentation
  {
  public:
    virtual bool setInternalData(SegmentationRepresentationSPtr rhs) = 0;

    virtual void addEditedRegion(const EspinaRegion &region, int cacheId = -1) = 0;

    /// Whether output has been manually edited
    virtual bool isEdited() const = 0;

    virtual void clearEditedRegions() = 0;

    /// Update output's edited region list
    virtual void commitEditedRegions(bool withData) const = 0;

    virtual void restoreEditedRegions(const QDir &cacheDir, const QString &outputId) = 0;

  protected:
    explicit SegmentationRepresentation(FilterOutput *output)
    : OutputRepresentation(output){}
  };
} // namespace EspINA

#endif // OUTPUTREPRESENTATION_H
