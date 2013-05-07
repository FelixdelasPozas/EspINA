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


#ifndef MARCHINGCUBEMESH_H
#define MARCHINGCUBEMESH_H

#include "Core/Outputs/MeshType.h"
#include <Core/Outputs/VolumeRepresentation.h>

#include <vtkSmartPointer.h>

class vtkDiscreteMarchingCubes;
class vtkImageConstantPad;

namespace EspINA
{
  class MarchingCubesMesh
  : public MeshType
  {
    Q_OBJECT
  public:
    explicit MarchingCubesMesh(SegmentationVolumeSPtr volume, FilterOutput *output = NULL);
    virtual ~MarchingCubesMesh();

    virtual bool setInternalData(SegmentationRepresentationSPtr rhs);

    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const;

    //virtual bool fetchSnapshot(Filter *filter, const QString &prefix);

    virtual bool isEdited() const;

    virtual bool isValid() const;

    virtual FilterOutput::EditedRegionSList editedRegions() const;

    virtual void clearEditedRegions();

    virtual void commitEditedRegions(bool withData) const;

    virtual void restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix);

    virtual vtkAlgorithmOutput *mesh() const;

  private slots:
    void updateMesh();

  private:
    SegmentationVolumeSPtr m_volume;

    vtkSmartPointer<vtkImageConstantPad>      m_pad;
    vtkSmartPointer<vtkDiscreteMarchingCubes> m_marchingCubes;
  };

} // namespace EspINA

#endif // MARCHINGCUBEMESH_H