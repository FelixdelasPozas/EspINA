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
#include <Core/Model/VolumeOutputType.h>

// ITK
#include <itkVTKImageImport.h>

// VTK
#include <vtkSmartPointer.h>

class vtkImageExport;
class vtkImplicitPolyDataDistance;

namespace EspINA
{
  class AppositionSurfaceVolume
  : public SegmentationVolumeType
  {
    public:
      typedef boost::shared_ptr<AppositionSurfaceVolume> Pointer;

    public:
      explicit AppositionSurfaceVolume(AppositionSurfaceFilter *filter);
      virtual ~AppositionSurfaceVolume();

      /// Volume's voxel's index at given spatial position
      /// It doesn't check whether the index is valid or not
      virtual itkVolumeType::IndexType index(Nm x, Nm y, Nm z);

      /// Get the vtk-equivalent extent defining the volume
      virtual void extent(int out[6]) const;
      /// Get the vtk-equivalent bounds defining the volume
      virtual void bounds(double out[6]) const;
      ///
      virtual void spacing(double out[3]) const;

      virtual itkVolumeIterator iterator();
      virtual itkVolumeIterator iterator(const EspinaRegion &region);

      virtual itkVolumeConstIterator constIterator();
      virtual itkVolumeConstIterator constIterator(const EspinaRegion &region);

      virtual itkVolumeType::Pointer toITK();
      virtual const itkVolumeType::Pointer toITK() const;

      virtual vtkAlgorithmOutput *toVTK();
      virtual const vtkAlgorithmOutput *toVTK() const;

      virtual vtkAlgorithmOutput *toMesh();

      virtual void update();

      /// Expands the volume to contain @region.
      virtual void expandToFitRegion(EspinaRegion region);

      /// Reduce volume dimensions to adjust it to the bounding box of the
      /// contained segmentation
      virtual bool fitToContent() throw (itk::ExceptionObject);

    protected:
      void rasterize(double *bounds) const;
      void transformVTK2ITK() const;

    private:

      // private attributes
      mutable vtkSmartPointer<vtkAlgorithmOutput>          m_vtkVolume;

      AppositionSurfaceFilter                             *m_filter;

      mutable vtkSmartPointer<vtkImageData>                m_emptyImage;
      mutable vtkSmartPointer<vtkImplicitPolyDataDistance> m_distance;

      typedef itk::VTKImageImport<itkVolumeType> itkImageImporter;
      mutable itkImageImporter::Pointer                    m_itkImporter;
      mutable vtkSmartPointer<vtkImageExport>              m_vtkExporter;

      mutable unsigned long int m_rasterizationTime;
      mutable double            m_rasterizationBounds[6];
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACEVOLUME_H_ */
