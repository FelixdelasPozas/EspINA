/*
 * AppositionSurfaceVolume.cpp
 *
 *  Created on: Jan 21, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceVolume.h"

// EspINA
#include <Core/EspinaTypes.h>

// VTK
#include <vtkImageExport.h>
#include <vtkAlgorithmOutput.h>
#include <vtkMath.h>
#include <vtkImplicitPolyDataDistance.h>


// ITK
#include <itkVTKImageImport.h>

// Qt
#include <QDebug>

namespace EspINA
{
  
  //----------------------------------------------------------------------------
  AppositionSurfaceVolume::AppositionSurfaceVolume(AppositionSurfaceFilter *filter)
  : SegmentationVolume(NULL)
  , m_vtkVolume(NULL)
  , m_filter(filter)
  , m_emptyImage(NULL)
  , m_distance(NULL)
  , m_itkImporter(NULL)
  , m_vtkExporter(NULL)
  {
  }
  
  //----------------------------------------------------------------------------
  AppositionSurfaceVolume::~AppositionSurfaceVolume()
  {
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::rasterize(double *imageBounds) const
  {
    itkVolumeType::SpacingType spacing = m_filter->getOriginSpacing();
    double minSpacing = std::min(spacing[0], std::min(spacing[1], spacing[2]));

    double bounds[6];
    if (imageBounds != NULL)
      memcpy(bounds, imageBounds, 6*sizeof(double));
    else
      m_filter->m_ap->GetBounds(bounds);

    int extent[6] = { vtkMath::Round(bounds[0]/spacing[0]),
                      vtkMath::Round(bounds[1]/spacing[0]),
                      vtkMath::Round(bounds[2]/spacing[1]),
                      vtkMath::Round(bounds[3]/spacing[1]),
                      vtkMath::Round(bounds[4]/spacing[2]),
                      vtkMath::Round(bounds[5]/spacing[2]) };

    m_emptyImage = vtkSmartPointer<vtkImageData>::New();
    m_emptyImage->SetSpacing(spacing[0], spacing[1], spacing[2]);
    m_emptyImage->SetExtent(extent);
    m_emptyImage->SetScalarTypeToUnsignedChar();
    m_emptyImage->AllocateScalars();
    m_emptyImage->Update();

    memset(m_emptyImage->GetScalarPointer(), SEG_BG_VALUE, m_emptyImage->GetNumberOfPoints());

    m_distance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    m_distance->SetInput(m_filter->m_ap);

    for (int x = extent[0]; x <= extent[1]; x++)
      for (int y = extent[2]; y <= extent[3]; y++)
        for (int z = extent[4]; z <= extent[5]; z++)
        {
          double point[3] = { (x+0.5)*spacing[0], (y+0.5)*spacing[1], (z+0.5)*spacing[2] };
          if (std::abs(m_distance->EvaluateFunction(point)) <= minSpacing)
          {
            unsigned char *pixel = reinterpret_cast<unsigned char*>(m_emptyImage->GetScalarPointer(x,y,z));
            *pixel = SEG_VOXEL_VALUE;
          }
        }

    m_vtkVolume = m_emptyImage->GetProducerPort();
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK()
  {
    if (m_vtkVolume == NULL)
      rasterize(NULL);

    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  const vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK() const
  {
    if (m_vtkVolume == NULL)
      rasterize(NULL);

    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  itkVolumeType::Pointer AppositionSurfaceVolume::toITK()
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return m_volume;
  }

  //----------------------------------------------------------------------------
  const itkVolumeType::Pointer AppositionSurfaceVolume::toITK() const
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return m_volume;
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::transformVTK2ITK() const
  {
    if (m_itkImporter.IsNotNull())
    {
      m_volume->Update();
      return;
    }

    if (NULL == m_vtkVolume)
      toVTK();

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
    m_volume->Update();

    Q_ASSERT(!m_volume.IsNull());
  }

  //----------------------------------------------------------------------------
  itkVolumeType::IndexType AppositionSurfaceVolume::index(Nm x, Nm y, Nm z)
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::index(x,y,z);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::extent(int out[6]) const
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::extent(out);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::bounds(double out[6]) const
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::bounds(out);
  }


  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::spacing(double out[3]) const
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::spacing(out);
  }

  //----------------------------------------------------------------------------
  itkVolumeIterator AppositionSurfaceVolume::iterator(const EspinaRegion &region)
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::iterator(region);
  }

  //----------------------------------------------------------------------------
  itkVolumeConstIterator AppositionSurfaceVolume::constIterator(const EspinaRegion &region)
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::constIterator(region);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::update()
  {
    if (m_volume.IsNull())
      transformVTK2ITK();
    else
      m_volume->Update();
  }

  typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::expandToFitRegion(EspinaRegion region)
  {
    double bounds[6];
    region.bounds(bounds);
    rasterize(bounds);

    if (!m_itkImporter.IsNull())
    {
      m_itkImporter = NULL;
      transformVTK2ITK();
    }
  }

  //----------------------------------------------------------------------------
  bool AppositionSurfaceVolume::strechToFitContent()
  {
    if (m_volume.IsNull())
      transformVTK2ITK();

    return SegmentationVolume::strechToFitContent();
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toMesh()
  {
    return m_filter->m_ap->GetProducerPort();
  }

} /* namespace EspINA */
