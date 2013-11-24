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

#ifndef ESPINA_STREAMED_VOLUME_H
#define ESPINA_STREAMED_VOLUME_H

#include "Core/Analysis/Data/VolumetricData.h"
#include "Core/Analysis/Data/VolumetricDataUtils.h"
#include "Core/Utils/BinaryMask.h"
#include "Core/Utils/Bounds.h"

#include <itkImageRegionIterator.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>

namespace EspINA {

  template class VolumetricData<itk::Image<unsigned char, 3>>;

  /** \brief Volume representation intended to save memory and speed up
   *  edition opertaions
   *
   *  Those voxel which don't belong to any block are assigned the value
   *  defined as background value.
   *
   *  Add operation will replace every voxel with a value different to 
   *  the background value.
   *  Sub operation will 
   */
  template<typename T>
  class StreamedVolume
  : public VolumetricData<T>
  {
  public:
    struct File_Not_Found_Exception{};

  public:
    explicit StreamedVolume();

    explicit StreamedVolume(const QFileInfo& fileName);

    void setFileName(const QFileInfo& fileName);

    QFileInfo fileName() const
    { return m_fileName; }

    virtual double memoryUsage() const;

    virtual const Bounds bounds() const;

    virtual void setOrigin(const NmVector3& origin);

    virtual NmVector3 origin() const
    { return m_origin; }

    virtual void setSpacing(const NmVector3& spacing);

    virtual NmVector3 spacing() const;

    virtual const typename T::Pointer itkImage() const;

    virtual const typename T::Pointer itkImage(const Bounds& bounds) const;

    virtual void draw(const vtkImplicitFunction*  brush,
                      const Bounds&               bounds,
                      const typename T::ValueType value) {}

    virtual void draw(const typename T::Pointer volume,
                      const Bounds&             bounds = Bounds()){}

    virtual void draw(const typename T::IndexType index,
                      const typename T::PixelType value = SEG_VOXEL_VALUE){}


    virtual void fitToContent(){}

    virtual void resize(const Bounds &bounds) {}

    virtual void undo() {}

    virtual bool isValid() const
    { return QFileInfo(m_fileName).exists(); }

    virtual bool fetchData(TemporalStorageSPtr storage, const QString& prefix)
    { return false; }

    virtual Snapshot snapshot(TemporalStorageSPtr storage, const QString& prefix) const
    { return Snapshot(); }

    virtual Snapshot editedRegionsSnapshot() const { return Snapshot(); }

  protected:
    typedef itk::ImageRegionIterator<T> ImageIterator;

  private:
    NmVector3 m_origin;
    NmVector3 m_spacing;

    Bounds  m_bounds;
    QString m_fileName;
    QString m_storageFileName;
  };

  #include "StreamedVolume.cpp"
}


#endif // ESPINA_STREAMED_VOLUME_H
