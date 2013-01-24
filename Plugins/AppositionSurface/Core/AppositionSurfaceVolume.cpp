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
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageExport.h>
#include <vtkAlgorithmOutput.h>

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
  , m_mesh2stencil(NULL)
  , m_stencil2image(NULL)
  , m_itkImporter(NULL)
  , m_vtkExporter(NULL)
  {
    // TODO: remove this to make it lazy
    rasterize();
    transformVtk2Itk();
  }
  
  //----------------------------------------------------------------------------
  AppositionSurfaceVolume::~AppositionSurfaceVolume()
  {
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::rasterize() const
  {
    if (m_emptyImage != NULL)
    {
      m_stencil2image->Update();
      return;
    }
    itkVolumeType::SpacingType spacing = m_filter->getOriginSpacing();
    itkVolumeType::RegionType region = m_filter->getOriginRegion();
    itkVolumeType::IndexType index = region.GetIndex();
    itkVolumeType::SizeType size = region.GetSize();
    double maxSpacing = std::max(spacing[0], std::max(spacing[1], spacing[2]));

    m_emptyImage = vtkSmartPointer<vtkImageData>::New();
    m_emptyImage->SetSpacing(spacing[0], spacing[1], spacing[2]);
    m_emptyImage->SetExtent(index[0], index[0]+size[0], index[1], index[1]+size[1], index[2], index[2]+size[2]);
    m_emptyImage->SetScalarTypeToUnsignedChar();
    m_emptyImage->AllocateScalars();

    m_mesh2stencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    m_mesh2stencil->SetTolerance(maxSpacing);
    m_mesh2stencil->SetInputConnection(m_filter->m_ap->GetProducerPort());
    m_mesh2stencil->Update();

    m_stencil2image = vtkSmartPointer<vtkImageStencil>::New();
    m_stencil2image->SetBackgroundValue(SEG_BG_VALUE);
    m_stencil2image->SetInputConnection(m_emptyImage->GetProducerPort());
    m_stencil2image->SetStencil(m_mesh2stencil->GetOutput());
    m_stencil2image->Update();

    m_vtkVolume = m_stencil2image->GetOutputPort();
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK()
  {
    qDebug() << "entrada en toVTK" << "vtkVolume" << m_vtkVolume;
    if (m_vtkVolume == NULL)
      rasterize();

    qDebug() << "salida de toVTK" << "vtkVolume" << m_vtkVolume;

    if (m_vtkVolume != NULL)
      m_vtkVolume->Print(std::cout);
    else
      qDebug() << "vtkVolume es NULL!!!!!!";
    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  const vtkAlgorithmOutput *AppositionSurfaceVolume::toVTK() const
  {
    qDebug() << "entrada en const toVTK" << "vtkVolume" << m_vtkVolume;
    if (m_vtkVolume == NULL)
      rasterize();

    qDebug() << "salida de const toVTK" << "vtkVolume" << m_vtkVolume;

    if (m_vtkVolume != NULL)
      m_vtkVolume->Print(std::cout);
    else
      qDebug() << "vtkVolume es NULL!!!!!!";

    return m_vtkVolume.GetPointer();
  }

  //----------------------------------------------------------------------------
  itkVolumeType::Pointer AppositionSurfaceVolume::toITK()
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return m_volume;
  }

  //----------------------------------------------------------------------------
  const itkVolumeType::Pointer AppositionSurfaceVolume::toITK() const
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return m_volume;
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::transformVtk2Itk() const
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

    if (m_volume.IsNull())
      Q_ASSERT(false);
  }

  //----------------------------------------------------------------------------
  itkVolumeType::IndexType AppositionSurfaceVolume::index(Nm x, Nm y, Nm z)
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::index(x,y,z);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::extent(int out[6]) const
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::extent(out);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::bounds(double out[6]) const
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::bounds(out);
  }


  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::spacing(double out[3]) const
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::spacing(out);
  }

  //----------------------------------------------------------------------------
  itkVolumeIterator AppositionSurfaceVolume::iterator(const EspinaRegion &region)
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::iterator(region);
  }

  //----------------------------------------------------------------------------
  itkVolumeConstIterator AppositionSurfaceVolume::constIterator(const EspinaRegion &region)
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return EspinaVolume::constIterator(region);
  }

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::update()
  {
    if (m_volume.IsNull())
      transformVtk2Itk();
    else
      m_volume->Update();
  }

  typedef itk::ImageRegionExclusionIteratorWithIndex<itkVolumeType> ExclusionIterator;

  //----------------------------------------------------------------------------
  void AppositionSurfaceVolume::expandToFitRegion(EspinaRegion region)
  {
    itkVolumeType::SpacingType spacing = m_filter->getOriginSpacing();
    double bounds[6];
    region.bounds(bounds);

    if (m_volume.IsNull())
    {
      m_emptyImage = vtkSmartPointer<vtkImageData>::New();
      m_emptyImage->SetSpacing(spacing[0], spacing[1], spacing[2]);
      m_emptyImage->SetExtent(bounds[0]/spacing[0],
                              bounds[1]/spacing[0],
                              bounds[2]/spacing[1],
                              bounds[3]/spacing[1],
                              bounds[4]/spacing[2],
                              bounds[5]/spacing[2]);
      m_emptyImage->AllocateScalars();

      m_mesh2stencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
      m_mesh2stencil->SetInputConnection(m_filter->m_ap->GetProducerPort());
      m_mesh2stencil->Update();

      m_stencil2image = vtkSmartPointer<vtkImageStencil>::New();
      m_stencil2image->SetBackgroundValue(SEG_BG_VALUE);
      m_stencil2image->SetInputConnection(m_emptyImage->GetProducerPort());
      m_stencil2image->SetStencil(m_mesh2stencil->GetOutput());
      m_stencil2image->Update();

      m_vtkVolume = m_stencil2image->GetOutputPort();

      transformVtk2Itk();
    }
    else
    {
      m_emptyImage->SetExtent(bounds[0]/spacing[0],
                              bounds[1]/spacing[0],
                              bounds[2]/spacing[1],
                              bounds[3]/spacing[1],
                              bounds[4]/spacing[2],
                              bounds[5]/spacing[2]);
      m_emptyImage->AllocateScalars();
      m_emptyImage->Modified();
      m_volume->Update();
    }
  }

  //----------------------------------------------------------------------------
  bool AppositionSurfaceVolume::strechToFitContent()
  {
    if (m_volume.IsNull())
      transformVtk2Itk();

    return SegmentationVolume::strechToFitContent();
  }

  //----------------------------------------------------------------------------
  vtkAlgorithmOutput *AppositionSurfaceVolume::toMesh()
  {
    return m_filter->m_ap->GetProducerPort();
  }

  //----------------------------------------------------------------------------
  AppositionSurfaceVolume AppositionSurfaceVolume::operator=(itkVolumeType::Pointer volume)
  {
    m_volume = volume;
    m_volume->DisconnectPipeline();

    return *this;
  }

} /* namespace EspINA */
