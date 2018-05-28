/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include "SplitFilter.h"
#include <Core/Types.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/Spatial.h>

// ITK
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

// VTK
#include <vtkImageStencilData.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageToImageStencil.h>
#include <vtkMath.h>

// Qt
#include <QDir>

using namespace ESPINA;

//-----------------------------------------------------------------------------
SplitFilter::SplitFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_ignoreCurrentOutputs(false)
, m_stencil(nullptr)
{
}

//-----------------------------------------------------------------------------
SplitFilter::~SplitFilter()
{
}

//-----------------------------------------------------------------------------
void SplitFilter::execute()
{
  reportProgress(0);
  if (!canExecute()) return;

  Q_ASSERT(m_inputs.size() == 1);
  auto input  = m_inputs[0]->output();
  auto volume = readLockVolume(input);

  if (!m_stencil && !fetchCacheStencil())
  {
    if (m_outputs.isEmpty())
    {
      Q_ASSERT(false);
    }
    else
    {
      // read-only filter.
      return;
    }
  }

  auto itkVolume = volume->itkImage();
  auto itkRegion = itkVolume->GetLargestPossibleRegion();

  auto split1 = itkVolumeType::New();
  split1->SetRegions(itkVolume->GetLargestPossibleRegion());
  split1->SetSpacing(itkVolume->GetSpacing());
  split1->SetOrigin(itkVolume->GetOrigin());
  split1->Allocate();
  split1->Update();

  auto split2 = itkVolumeType::New();
  split2->SetRegions(itkVolume->GetLargestPossibleRegion());
  split2->SetSpacing(itkVolume->GetSpacing());
  split2->SetOrigin(itkVolume->GetOrigin());
  split2->Allocate();
  split2->Update();

  reportProgress(25);
  if (!canExecute()) return;

  int shift[3]; // Stencil origin differ from creation to fetch

  double origin[3];
  double spacing[3];

  m_stencil->GetOrigin(origin);
  m_stencil->GetSpacing(spacing);

  for (int i = 0; i < 3; ++i)
  {
    shift[i] = vtkMath::Round(origin[i] / spacing[i]);
  }

  bool hasVoxels1 = false;
  bool hasVoxels2 = false;

  itk::ImageRegionConstIterator<itkVolumeType> it(itkVolume, itkVolume->GetLargestPossibleRegion());
  itk::ImageRegionIterator<itkVolumeType> split1it(split1, split1->GetLargestPossibleRegion());
  itk::ImageRegionIterator<itkVolumeType> split2it(split2, split2->GetLargestPossibleRegion());

  it.GoToBegin();
  split1it.GoToBegin();
  split2it.GoToBegin();

  for(; !it.IsAtEnd(); ++it, ++split1it, ++split2it)
  {
    auto value = it.Value();
    itkVolumeType::IndexType index = it.GetIndex();
    if (m_stencil->IsInside(index[0] - shift[0], index[1] - shift[1], index[2] - shift[2]))
    {
      split1it.Set(value);
      split2it.Set(SEG_BG_VALUE);
      hasVoxels1 |= (value == SEG_VOXEL_VALUE);
    }
    else
    {
      split1it.Set(SEG_BG_VALUE);
      split2it.Set(value);
      hasVoxels2 |= (value == SEG_VOXEL_VALUE);
    }
  }

  reportProgress(50);
  if (!canExecute()) return;

  itkVolumeType::Pointer volumes[2]{split1, split2};

  if (hasVoxels1 && hasVoxels2)
  {
    auto spacing = input->spacing();

    for(auto i: {0, 1})
    {
      auto bounds = minimalBounds<itkVolumeType>(volumes[i], SEG_BG_VALUE);
      auto volume = std::make_shared<SparseVolume<itkVolumeType>>(bounds, spacing);
      
      volume->draw(volumes[i], bounds);

      if (!m_outputs.contains(i))
      {
        m_outputs[i] = std::make_shared<Output>(this, i, spacing);
      }
      m_outputs[i]->setData(volume);
      m_outputs[i]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[i].get()));
      m_outputs[i]->setSpacing(spacing);

      reportProgress(75 + 25*i);
      if (!canExecute()) return;
    }

    m_ignoreCurrentOutputs = false;
  }
}

//----------------------------------------------------------------------------
bool SplitFilter::ignoreStorageContent() const
{
  return false;
}

//----------------------------------------------------------------------------
bool SplitFilter::needUpdate() const
{
  return !validOutput(0) || ignoreStorageContent();
}

//-----------------------------------------------------------------------------
bool SplitFilter::fetchCacheStencil() const
{
  if (m_ignoreCurrentOutputs)
  {
    return false;
  }

  bool returnVal = false;

  if (storage()->exists(stencilFile()))
  {
    QString fileName = storage()->absoluteFilePath(stencilFile());
    const QString utfFilename = fileName.toUtf8();
    const QString asciiFilename = utfFilename.toAscii();

    auto stencilReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    stencilReader->SetFileName(asciiFilename.toStdString().c_str());
    stencilReader->ReadAllFieldsOn();
    stencilReader->Update();

    auto image = vtkImageData::SafeDownCast(stencilReader->GetOutput());
    auto spacing = output(0)->spacing();
    image->SetSpacing(spacing[0], spacing[1], spacing[2]);
    image->Modified();

    auto convert = vtkSmartPointer<vtkImageToImageStencil>::New();
    convert->SetInputData(image);
    convert->ThresholdBetween(1,1);
    convert->Update();

    m_stencil = vtkSmartPointer<vtkImageStencilData>::New();
    m_stencil->DeepCopy(convert->GetOutput());

    changeStencilSpacing(output(0)->spacing());

    returnVal = true;
  }

  return returnVal;
}

//-----------------------------------------------------------------------------
void SplitFilter::changeStencilSpacing(const NmVector3& spacing) const
{
  if(areEqual(spacing[0], 0) || areEqual(spacing[1], 0) || areEqual(spacing[2], 0))
  {
    qWarning() << name() << "(" << uuid() << ") attempting to use invalid spacing" << spacing;
  }
  else
  {
    double stencilSpacing[3], stencilOrigin[3];
    m_stencil->GetSpacing(stencilSpacing);

    if((spacing[0] == stencilSpacing[0]) &&
       (spacing[1] == stencilSpacing[1]) &&
       (spacing[2] == stencilSpacing[2]))
    {
      return;
    }

    m_stencil->GetOrigin(stencilOrigin);

    NmVector3 ratio{spacing[0]/stencilSpacing[0],
                    spacing[1]/stencilSpacing[1],
                    spacing[2]/stencilSpacing[2]};

    for (int i = 0; i < 3; ++i)
    {
      stencilOrigin[i] *= ratio[i];
      stencilSpacing[i] = spacing[i];
    }

    m_stencil->SetOrigin(stencilOrigin);
    m_stencil->SetSpacing(stencilSpacing);
  }

  m_stencil->Modified();
}

//-----------------------------------------------------------------------------
void SplitFilter::setStencil(vtkSmartPointer<vtkImageStencilData> stencil)
{
  m_stencil = stencil;
  m_ignoreCurrentOutputs = true;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageStencilData> SplitFilter::stencil() const
{
  if (!m_stencil)
  {
    fetchCacheStencil();
  }

  return m_stencil;
}

//-----------------------------------------------------------------------------
void SplitFilter::changeSpacing(const NmVector3& origin, const NmVector3& spacing)
{
  if (m_stencil)
  {
    changeStencilSpacing(spacing);
  }

  Filter::changeSpacing(origin, spacing);
}

//-----------------------------------------------------------------------------
Snapshot SplitFilter::saveFilterSnapshot() const
{
  Snapshot snapshot;

  // NOTE: not aborting if the file is not found fixes a bug in earlier versions that didn't store
  // the vti file in the seg, this renders this filter a read-only filter, that is, cannot be executed
  // again.
  if(m_stencil || fetchCacheStencil())
  {
    auto convert = vtkSmartPointer<vtkImageStencilToImage>::New();
    convert->SetInputData(m_stencil);
    convert->SetInsideValue(1);
    convert->SetOutsideValue(0);
    convert->SetOutputScalarTypeToUnsignedChar();
    convert->Update();

    auto stencilImage = vtkSmartPointer<vtkImageData>::New();
    stencilImage->DeepCopy(convert->GetOutput());
    stencilImage->SetOrigin(m_stencil->GetOrigin());
    stencilImage->SetSpacing(m_stencil->GetSpacing());
    stencilImage->Modified();

    auto stencilWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
    stencilWriter->SetInputData(stencilImage);
    stencilWriter->SetFileTypeToBinary();
    stencilWriter->SetWriteToOutputString(true);
    stencilWriter->Write();

    snapshot << SnapshotData{stencilFile(), QByteArray{stencilWriter->GetOutputString(), stencilWriter->GetOutputStringLength()}};
  }

  return snapshot;
}
