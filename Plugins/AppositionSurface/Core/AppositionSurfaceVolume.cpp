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
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

// Qt
#include <QDebug>

namespace EspINA
{
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;

  //----------------------------------------------------------------------------
  AppositionSurfaceVolume::AppositionSurfaceVolume(AppositionSurfaceFilter *filter)
  : SegmentationVolume(NULL)
  , m_vtkVolume(NULL)
  , m_filter(filter)
  , m_emptyImage(NULL)
  , m_distance(NULL)
  , m_itkImporter(NULL)
  , m_vtkExporter(NULL)
  , m_rasterizationTime(0)
  {
    memset(m_rasterizationBounds, 0, 6*sizeof(double));
  }
  
  //----------------------------------------------------------------------------
  AppositionSurfaceVolume::~AppositionSurfaceVolume()
  {
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::rasterize(double *imageBounds) const
  {
    if (m_vtkVolume != NULL &&
        m_rasterizationTime == m_filter->m_ap->GetMTime() &&
        memcmp(imageBounds, m_rasterizationBounds, 6*sizeof(double)) == 0)
    {
      return;
    }

    itkVolumeType::SpacingType vSpacing = m_filter->getOriginSpacing();
    double minSpacing = std::min(vSpacing[0], std::min(vSpacing[1], vSpacing[2]));

    if (imageBounds != NULL)
    {
      memcpy(m_rasterizationBounds, imageBounds, 6*sizeof(double));
    }
    else
        m_filter->m_ap->GetBounds(m_rasterizationBounds);

    int extent[6] = { vtkMath::Round(m_rasterizationBounds[0]/vSpacing[0]),
                      vtkMath::Round(m_rasterizationBounds[1]/vSpacing[0]),
                      vtkMath::Round(m_rasterizationBounds[2]/vSpacing[1]),
                      vtkMath::Round(m_rasterizationBounds[3]/vSpacing[1]),
                      vtkMath::Round(m_rasterizationBounds[4]/vSpacing[2]),
                      vtkMath::Round(m_rasterizationBounds[5]/vSpacing[2]) };

    m_emptyImage = vtkSmartPointer<vtkImageData>::New();
    m_emptyImage->SetSpacing(vSpacing[0], vSpacing[1], vSpacing[2]);
    m_emptyImage->SetExtent(extent);
    m_emptyImage->SetScalarTypeToUnsignedChar();
    m_emptyImage->AllocateScalars();
    m_emptyImage->Update();

    memset(m_emptyImage->GetScalarPointer(), SEG_BG_VALUE, m_emptyImage->GetNumberOfPoints());

    m_distance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    m_distance->SetInput(m_filter->m_ap);
    m_distance->SetTolerance(0);

    for (int x = extent[0]; x <= extent[1]; x++)
      for (int y = extent[2]; y <= extent[3]; y++)
        for (int z = extent[4]; z <= extent[5]; z++)
        {
          double point[3] = { x*vSpacing[0], y*vSpacing[1], z*vSpacing[2] };
          if (std::abs(m_distance->EvaluateFunction(point)) <= minSpacing)
          {
            unsigned char *pixel = reinterpret_cast<unsigned char*>(m_emptyImage->GetScalarPointer(x,y,z));
            *pixel = SEG_VOXEL_VALUE;
          }
        }

    m_rasterizationTime = m_filter->m_ap->GetMTime();
    m_vtkVolume = m_emptyImage->GetProducerPort();
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::toVTK();

    if ((m_vtkVolume == NULL) || (m_filter->m_ap->GetMTime() != m_rasterizationTime))
      rasterize(m_filter->m_ap->GetBounds());

    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  const vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK() const
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::toVTK();

    if ((m_vtkVolume == NULL) || (m_filter->m_ap->GetMTime() != m_rasterizationTime))
      rasterize(m_filter->m_ap->GetBounds());

    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  itkVolumeType::Pointer AppositionSurfaceVolume::toITK()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::toITK();

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
  const itkVolumeType::Pointer AppositionSurfaceVolume::toITK() const
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::toITK();

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
  void AppositionSurfaceVolume::transformVTK2ITK() const
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
  itkVolumeType::IndexType AppositionSurfaceVolume::index(Nm x, Nm y, Nm z)
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::index(x,y,z);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::index(x,y,z);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::extent(int out[6]) const
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::extent(out);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::extent(out);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::bounds(double out[6]) const
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::bounds(out);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::bounds(out);
  }


  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::spacing(double out[3]) const
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::spacing(out);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::spacing(out);
  }

  //----------------------------------------------------------------------------
  itkVolumeIterator AppositionSurfaceVolume::iterator()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::iterator(SegmentationVolume::espinaRegion());

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::iterator(espinaRegion());
  }

  //----------------------------------------------------------------------------
  itkVolumeIterator AppositionSurfaceVolume::iterator(const EspinaRegion &region)
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::iterator(region);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::iterator(region);
  }

  //----------------------------------------------------------------------------
  itkVolumeConstIterator AppositionSurfaceVolume::constIterator()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::constIterator(SegmentationVolume::espinaRegion());

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::constIterator(espinaRegion());
  }

  //----------------------------------------------------------------------------
  itkVolumeConstIterator AppositionSurfaceVolume::constIterator(const EspinaRegion &region)
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::constIterator(region);

    if (m_volume.IsNull())
      transformVTK2ITK();

    return EspinaVolume::constIterator(region);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::update()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::update();

    if (m_vtkVolume != NULL)
      toVTK();

    if (m_volume.IsNotNull())
      toITK();
  }

  typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::expandToFitRegion(EspinaRegion region)
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::expandToFitRegion(region);

    double bounds[6];
    region.bounds(bounds);
    rasterize(bounds);

    if (m_volume.IsNotNull())
      toITK();
  }

  //----------------------------------------------------------------------------
  bool AppositionSurfaceVolume::fitToContent()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::fitToContent();

    if (m_volume.IsNull())
      transformVTK2ITK();

    Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
    image2label->ReleaseDataFlagOn();
    image2label->SetInput(m_volume);
    image2label->Update();

    // Get segmentation's Bounding Box
    LabelMapType *labelMap = image2label->GetOutput();
    if (labelMap->GetNumberOfLabelObjects() == 0)
      return false;

    LabelObjectType *segmentation = labelMap->GetLabelObject(SEG_VOXEL_VALUE);
    LabelObjectType::RegionType segBB = segmentation->GetBoundingBox();
    LabelObjectType::RegionType::IndexType index = segBB.GetIndex();
    LabelObjectType::RegionType::SizeType size = segBB.GetSize();

    double volumeSpacing[3];
    spacing(volumeSpacing);

    double bounds[6] = { index[0] * volumeSpacing[0],
                         (index[0]+size[0]) * volumeSpacing[0],
                         index[1] * volumeSpacing[1],
                         (index[1]+size[1]) * volumeSpacing[1],
                         index[2] * volumeSpacing[2],
                         (index[2]+size[2]) * volumeSpacing[2] };

    rasterize(bounds);
    transformVTK2ITK();

    return true;
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toMesh()
  {
    if (m_filter->m_ap == NULL)
      return SegmentationVolume::toMesh();

    return m_filter->m_ap->GetProducerPort();
  }

} /* namespace EspINA */
