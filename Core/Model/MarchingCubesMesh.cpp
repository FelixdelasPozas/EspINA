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


#include "MarchingCubesMesh.h"

#include "Core/Outputs/RawVolume.h"

#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageData.h>
#include <vtkAlgorithmOutput.h>

using namespace EspINA;

//----------------------------------------------------------------------------
MarchingCubesMesh::MarchingCubesMesh(SegmentationVolumeSPtr volume,
                                     FilterOutput *output)
: MeshType(output)
, m_volume(volume)
{
  connect(volume.get(), SIGNAL(representationChanged()),
          this, SLOT(updateMesh()));
}

//----------------------------------------------------------------------------
MarchingCubesMesh::~MarchingCubesMesh()
{

}

//----------------------------------------------------------------------------
bool MarchingCubesMesh::setInternalData(SegmentationRepresentationSPtr rhs)
{
  return false;
}

//----------------------------------------------------------------------------
bool MarchingCubesMesh::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
{
  return false;
}

// //----------------------------------------------------------------------------
// bool MarchingCubesMesh::fetchSnapshot(Filter *filter, const QString &prefix)
// {
//   return segmentationVolume(m_output)->isValid();
// }

//----------------------------------------------------------------------------
bool MarchingCubesMesh::isEdited() const
{
  return false;
}

//----------------------------------------------------------------------------
bool MarchingCubesMesh::isValid() const
{
  return true;
}

//----------------------------------------------------------------------------
FilterOutput::EditedRegionSList MarchingCubesMesh::editedRegions() const
{
  FilterOutput::EditedRegionSList regions;
  return regions;
}

//----------------------------------------------------------------------------
void MarchingCubesMesh::clearEditedRegions()
{

}

//----------------------------------------------------------------------------
void MarchingCubesMesh::commitEditedRegions(bool withData) const
{

}


//----------------------------------------------------------------------------
void MarchingCubesMesh::restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix)
{

}

//----------------------------------------------------------------------------
vtkAlgorithmOutput *MarchingCubesMesh::mesh() const
{
  return m_marchingCubes->GetOutput()->GetProducerPort();
}

//----------------------------------------------------------------------------
void MarchingCubesMesh::updateMesh()
{
  vtkAlgorithmOutput *vtkVolume = segmentationVolume(m_output)->toVTK();

  int extent[6];
  vtkImageData *image = vtkImageData::SafeDownCast(vtkVolume->GetProducer()->GetOutputDataObject(0));
  image->GetExtent(extent);
  extent[0]--;
  extent[1]++;
  extent[2]--;
  extent[3]++;
  extent[4]--;
  extent[5]++;

  if (NULL == m_pad)
  {
    // segmentation image need to be padded to avoid segmentation voxels from touching the edges of the
    // image (and create morphologically correct actors)

    m_pad = vtkSmartPointer<vtkImageConstantPad>::New();
    m_pad->SetInputConnection(vtkVolume);
    m_pad->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
    m_pad->SetConstant(0);

    m_marchingCubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
    m_marchingCubes->ReleaseDataFlagOn();
    m_marchingCubes->SetNumberOfContours(1);
    m_marchingCubes->GenerateValues(1, 255, 255);
    m_marchingCubes->ComputeScalarsOff();
    m_marchingCubes->ComputeNormalsOff();
    m_marchingCubes->ComputeGradientsOff();
    m_marchingCubes->SetInputConnection(m_pad->GetOutputPort());
  }
  else
  {
    if (m_pad->GetInputConnection(0,0) != vtkVolume)
      m_pad->SetInputConnection(vtkVolume);

    int outputExtent[6];
    m_pad->GetOutputWholeExtent(outputExtent);
    if (memcmp(extent, outputExtent, 6*sizeof(int)) != 0)
    {
      m_pad->SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
      m_pad->Update();
    }
  }

  m_marchingCubes->Update();
}