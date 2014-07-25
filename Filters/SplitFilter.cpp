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
#include <Core/EspinaTypes.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>

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
  emit progress(0);
  if (!canExecute()) return;

  Q_ASSERT(m_inputs.size() == 1);
  auto volume = volumetricData(m_inputs.first()->output());

  if (nullptr == m_stencil && !fetchCacheStencil())
  {
    if (m_outputs.isEmpty())
      Q_ASSERT(false);
    else
      // read-only filter.
      return;
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

  emit progress(25);
  if (!canExecute()) return;

  itk::ImageRegionConstIterator<itkVolumeType> it(itkVolume, itkVolume->GetLargestPossibleRegion());
  itk::ImageRegionIterator<itkVolumeType> split1it(split1, split1->GetLargestPossibleRegion());
  itk::ImageRegionIterator<itkVolumeType> split2it(split2, split2->GetLargestPossibleRegion());

  it.GoToBegin();
  split1it.GoToBegin();
  split2it.GoToBegin();

  bool isEmpty1 = true;
  bool isEmpty2 = true;

  int shift[3]; // Stencil origin differ from creation to fetch
  for (int i = 0; i < 3; ++i)
    shift[i] = vtkMath::Round(m_stencil->GetOrigin()[i] / m_stencil->GetSpacing()[i]);

  for(; !it.IsAtEnd(); ++it, ++split1it, ++split2it)
  {
    auto value = it.Value();
    itkVolumeType::IndexType index = it.GetIndex();
    if (m_stencil->IsInside(index[0] - shift[0], index[1] - shift[1], index[2] - shift[2]))
    {
      split1it.Set(value);
      split2it.Set(SEG_BG_VALUE);
      if (isEmpty1)
        isEmpty1 = value != SEG_VOXEL_VALUE;
    }
    else
    {
      split1it.Set(SEG_BG_VALUE);
      split2it.Set(value);
      if (isEmpty2)
        isEmpty2 = value != SEG_VOXEL_VALUE;
    }
  }

  emit progress(50);
  if (!canExecute()) return;

  itkVolumeType::Pointer volumes[2]{split1, split2};

  if (!isEmpty1 && !isEmpty2)
  {
    for(auto i: {0, 1})
    {
      auto spacing = m_inputs.first()->output()->spacing();
      m_outputs[i] = OutputSPtr(new Output(this, i));
      auto bounds = minimalBounds<itkVolumeType>(volumes[i], SEG_BG_VALUE);

      DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(bounds, spacing)};
      volume->draw(volumes[i], bounds);

      MeshDataSPtr mesh{new MarchingCubesMesh<itkVolumeType>(volume)};

      m_outputs[i]->setData(volume);
      m_outputs[i]->setData(mesh);
      m_outputs[i]->setSpacing(spacing);

      emit progress(75 + 25*i);
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
bool SplitFilter::invalidateEditedRegions()
{
  return false;
}

//----------------------------------------------------------------------------
bool SplitFilter::needUpdate() const
{
  return m_outputs.isEmpty();
}

//----------------------------------------------------------------------------
bool SplitFilter::needUpdate(Output::Id id) const
{
  if (id != 0 && id != 1) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//----------------------------------------------------------------------------
void SplitFilter::execute(Output::Id id)
{
  Q_ASSERT(id == 0 || id == 1);
  execute();
}

//-----------------------------------------------------------------------------
bool SplitFilter::fetchCacheStencil() const
{
  if (m_ignoreCurrentOutputs)
    return false;

  bool returnVal = false;

  if (storage()->exists(stencilFile()))
  {
    QString fileName = storage()->absoluteFilePath(stencilFile());
    vtkSmartPointer<vtkGenericDataObjectReader> stencilReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    stencilReader->SetFileName(fileName.toStdString().c_str());
    stencilReader->ReadAllFieldsOn();
    stencilReader->Update();

    vtkSmartPointer<vtkImageToImageStencil> convert = vtkSmartPointer<vtkImageToImageStencil>::New();
    convert->SetInputData(vtkImageData::SafeDownCast(stencilReader->GetOutput()));
    convert->ThresholdBetween(1,1);
    convert->Update();

    m_stencil = vtkSmartPointer<vtkImageStencilData>(convert->GetOutput());

    returnVal = true;
  }

  return returnVal;
}

//-----------------------------------------------------------------------------
Snapshot SplitFilter::saveFilterSnapshot() const
{
  Snapshot snapshot;

  // next line loads the stencil file from disk (if it exists, see note)
  // NOTE: not aborting if the file is not found fixes a bug in earlier versions that didn't store
  // the vti file in the seg, this renders this filter a read-only filter, that is, cannot be executed
  // again.
  if(m_stencil != nullptr || fetchCacheStencil())
  {
    vtkSmartPointer<vtkImageStencilToImage> convert = vtkSmartPointer<vtkImageStencilToImage>::New();
    convert->SetInputData(m_stencil);
    convert->SetInsideValue(1);
    convert->SetOutsideValue(0);
    convert->SetOutputScalarTypeToUnsignedChar();
    convert->Update();

    vtkSmartPointer<vtkGenericDataObjectWriter> stencilWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
    stencilWriter->SetInputConnection(convert->GetOutputPort());
    stencilWriter->SetFileTypeToBinary();
    stencilWriter->SetWriteToOutputString(true);
    stencilWriter->Write();

    snapshot << SnapshotData{stencilFile(), QByteArray{stencilWriter->GetOutputString(), stencilWriter->GetOutputStringLength()}};
  }
  return snapshot;
}
