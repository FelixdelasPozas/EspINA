/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RasterizedVolume.h"

#include <vtkMath.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>

using namespace EspINA;

//----------------------------------------------------------------------------
RasterizedVolume::RasterizedVolume(vtkAlgorithmOutput *mesh,
                                   itkVolumeType::SpacingType spacing,
                                   FilterOutput *output)
: RawSegmentationVolume(output)
, m_mesh(mesh)
, m_spacing(spacing)
{
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer RasterizedVolume::toITK()
{
  Q_ASSERT(m_mesh);

  if (m_volume.IsNull() || (m_vtkExporter == NULL) || m_vtkExporter->GetInputConnection(0,0) != m_vtkVolume)
    transformVTK2ITK();
  else
    if (m_vtkVolume->GetMTime() != m_ITKGenerationTime)
    {
      m_volume->Update();
      m_ITKGenerationTime = m_vtkVolume->GetMTime();
    }

  return m_volume;
}

//----------------------------------------------------------------------------
const itkVolumeType::Pointer RasterizedVolume::toITK() const
{
  Q_ASSERT(m_mesh);

  if (m_volume.IsNull() || (m_vtkExporter == NULL) || m_vtkExporter->GetInputConnection(0,0) != m_vtkVolume)
    transformVTK2ITK();
  else
    if (m_vtkVolume->GetMTime() != m_ITKGenerationTime)
    {
      m_volume->Update();
      m_ITKGenerationTime = m_vtkVolume->GetMTime();
    }

  return m_volume;
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput *RasterizedVolume::toVTK()
{
  Q_ASSERT(m_mesh);

  if ((m_vtkVolume == NULL) || (m_mesh->GetMTime() != m_rasterizationTime))
  {
    vtkPolyData *mesh = dynamic_cast<vtkPolyData *>(m_mesh->GetProducer()->GetInputDataObject(0, 0));
    rasterize(mesh->GetBounds());
  }

  return m_vtkVolume.GetPointer();
}


//----------------------------------------------------------------------------
const vtkAlgorithmOutput *RasterizedVolume::toVTK() const
{
  Q_ASSERT(m_mesh);

  if ((m_vtkVolume == NULL) || (m_mesh->GetMTime() != m_rasterizationTime))
  {
    vtkPolyData *mesh = dynamic_cast<vtkPolyData *>(m_mesh->GetProducer()->GetInputDataObject(0, 0));
    rasterize(mesh->GetBounds());
  }

  return m_vtkVolume.GetPointer();
}

//----------------------------------------------------------------------------
void RasterizedVolume::rasterize(double *imageBounds) const
{
  if (m_vtkVolume != NULL &&
      m_rasterizationTime == m_mesh->GetMTime() &&
      memcmp(imageBounds, m_rasterizationBounds, 6*sizeof(double)) == 0)
  {
    return;
  }

  double minSpacing = std::min(m_spacing[0], std::min(m_spacing[1], m_spacing[2]));

  vtkPolyData *mesh = dynamic_cast<vtkPolyData *>(m_mesh->GetProducer()->GetInputDataObject(0, 0));
  if (imageBounds != NULL)
  {
    memcpy(m_rasterizationBounds, imageBounds, 6*sizeof(double));
  }
  else
  {
    mesh->GetBounds(m_rasterizationBounds);
  }

  int extent[6] = {
    vtkMath::Round(m_rasterizationBounds[0]/m_spacing[0]),
    vtkMath::Round(m_rasterizationBounds[1]/m_spacing[0]),
    vtkMath::Round(m_rasterizationBounds[2]/m_spacing[1]),
    vtkMath::Round(m_rasterizationBounds[3]/m_spacing[1]),
    vtkMath::Round(m_rasterizationBounds[4]/m_spacing[2]),
    vtkMath::Round(m_rasterizationBounds[5]/m_spacing[2])
  };

  m_emptyImage = vtkSmartPointer<vtkImageData>::New();
  m_emptyImage->SetSpacing(m_spacing[0], m_spacing[1], m_spacing[2]);
  m_emptyImage->SetExtent(extent);
  m_emptyImage->SetScalarTypeToUnsignedChar();
  m_emptyImage->AllocateScalars();
  m_emptyImage->Update();

  memset(m_emptyImage->GetScalarPointer(), SEG_BG_VALUE, m_emptyImage->GetNumberOfPoints());

  m_distance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
  m_distance->SetInput(mesh);
  m_distance->SetTolerance(0);

  for (int x = extent[0]; x <= extent[1]; x++)
    for (int y = extent[2]; y <= extent[3]; y++)
      for (int z = extent[4]; z <= extent[5]; z++)
      {
        double point[3] = { x*m_spacing[0], y*m_spacing[1], z*m_spacing[2] };
        if (std::abs(m_distance->EvaluateFunction(point)) <= minSpacing)
        {
          unsigned char *pixel = reinterpret_cast<unsigned char*>(m_emptyImage->GetScalarPointer(x,y,z));
          *pixel = SEG_VOXEL_VALUE;
        }
      }
      
      m_rasterizationTime = m_mesh->GetMTime();
      m_vtkVolume = m_emptyImage->GetProducerPort();
}

//----------------------------------------------------------------------------
void RasterizedVolume::transformVTK2ITK() const
{
  if (NULL == m_vtkVolume)
    toVTK();
  
  if (m_itkImporter.IsNotNull())
  {
    if (m_vtkExporter->GetInputConnection(0,0) != m_vtkVolume)
    {
      m_vtkExporter->SetInputConnection(m_vtkVolume);
      m_vtkExporter->Modified();
    }
    
    m_volume->Update();
  }
  else
  {
    m_itkImporter = itkImageImporter::New();
    m_vtkExporter = vtkImageExport::New();
    
    m_vtkExporter->SetInputConnection(m_vtkVolume);
    
    m_itkImporter->SetUpdateInformationCallback(m_vtkExporter->GetUpdateInformationCallback());
    m_itkImporter->SetPipelineModifiedCallback(m_vtkExporter->GetPipelineModifiedCallback());
    m_itkImporter->SetWholeExtentCallback(m_vtkExporter->GetWholeExtentCallback());
    m_itkImporter->SetSpacingCallback(m_vtkExporter->GetSpacingCallback());
    m_itkImporter->SetOriginCallback(m_vtkExporter->GetOriginCallback());
    m_itkImporter->SetScalarTypeCallback(m_vtkExporter->GetScalarTypeCallback());
    m_itkImporter->SetNumberOfComponentsCallback(m_vtkExporter->GetNumberOfComponentsCallback());
    m_itkImporter->SetPropagateUpdateExtentCallback(m_vtkExporter->GetPropagateUpdateExtentCallback());
    m_itkImporter->SetUpdateDataCallback(m_vtkExporter->GetUpdateDataCallback());
    m_itkImporter->SetDataExtentCallback(m_vtkExporter->GetDataExtentCallback());
    m_itkImporter->SetBufferPointerCallback(m_vtkExporter->GetBufferPointerCallback());
    m_itkImporter->SetCallbackUserData(m_vtkExporter->GetCallbackUserData());
    m_itkImporter->UpdateLargestPossibleRegion();
    
    m_volume = m_itkImporter->GetOutput();
  }
  
  m_volume->Update();
  m_ITKGenerationTime = m_vtkVolume->GetMTime();
}

//----------------------------------------------------------------------------
RasterizedVolumeSPtr EspINA::rasterizedVolume(OutputSPtr output)
{
  SegmentationOutputSPtr segmentationOutput = boost::dynamic_pointer_cast<SegmentationOutput>(output);
  Q_ASSERT(segmentationOutput.get());
  return boost::dynamic_pointer_cast<RasterizedVolume>(segmentationOutput->representation(SegmentationVolume::TYPE));
}


