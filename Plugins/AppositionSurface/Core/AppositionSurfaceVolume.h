/*
 * AppositionSurfaceVolume.h
 *
 *  Created on: Jan 21, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACEVOLUME_H_
#define APPOSITIONSURFACEVOLUME_H_

#include "../Filter/AppositionSurfaceFilter.h"

// EspINA
#include <Core/EspinaVolume.h>

// ITK
#include <itkVTKImageImport.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyDataToImageStencil;
class vtkImageStencil;
class vtkImageExport;

namespace EspINA
{
  class AppositionSurfaceVolume
  : public SegmentationVolume
  {
    public:
      typedef boost::shared_ptr<AppositionSurfaceVolume> Pointer;

    public:
      explicit AppositionSurfaceVolume(AppositionSurfaceFilter *filter);
      virtual ~AppositionSurfaceVolume();

      //AppositionSurfaceVolume operator=(itkVolumeType::Pointer volume);

      /// Volume's voxel's index at given spatial position
      /// It doesn't check whether the index is valid or not
      itkVolumeType::IndexType index(Nm x, Nm y, Nm z);

      /// Get the vtk-equivalent extent defining the volume
      void extent(int out[6]) const;
      /// Get the vtk-equivalent bounds defining the volume
      void bounds(double out[6]) const;
      ///
      void spacing(double out[3]) const;

      itkVolumeIterator iterator();
      itkVolumeIterator iterator(const EspinaRegion &region);

      itkVolumeConstIterator constIterator();
      itkVolumeConstIterator constIterator(const EspinaRegion &region);

      itkVolumeType::Pointer toITK();
      const itkVolumeType::Pointer toITK() const;

      vtkAlgorithmOutput *toVTK();
      const vtkAlgorithmOutput *toVTK() const;

      vtkAlgorithmOutput *toMesh();

      void update();

      /// Expands the volume to contain @region.
      void expandToFitRegion(EspinaRegion region);

      /// Reduce volume dimensions to adjust it to the bounding box of the
      /// contained segmentation
      bool strechToFitContent();

    protected:
      void rasterize() const;
      void transformVtk2Itk() const;

    private:
      // not allowed
      void setVolume(itkVolumeType::Pointer volume, bool disconnect=false);

      // private attributes
      mutable vtkSmartPointer<vtkAlgorithmOutput>        m_vtkVolume;

      AppositionSurfaceFilter                           *m_filter;

      mutable vtkSmartPointer<vtkImageData>              m_emptyImage;
      mutable vtkSmartPointer<vtkPolyDataToImageStencil> m_mesh2stencil;
      mutable vtkSmartPointer<vtkImageStencil>           m_stencil2image;

      typedef itk::VTKImageImport<itkVolumeType> itkImageImporter;
      mutable itkImageImporter::Pointer                  m_itkImporter;
      mutable vtkSmartPointer<vtkImageExport>            m_vtkExporter;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACEVOLUME_H_ */
