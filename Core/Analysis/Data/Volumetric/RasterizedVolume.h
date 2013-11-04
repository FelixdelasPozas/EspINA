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

#ifndef ESPINA_RASTERIZEDVOLUME_H
#define ESPINA_RASTERIZEDVOLUME_H

#include "EspinaCore_Export.h"

#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/Volumetric/RawVolume.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/Mesh/RawMesh.h>

// ITK
#include <itkVTKImageImport.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkImageExport.h>

namespace EspINA
{
  template<class T>
  class EspinaCore_EXPORT RasterizedVolume
  : public RawVolume<T>
  {
  public:
    explicit RasterizedVolume(MeshDataSPtr  mesh,
                              OutputSPtr    output = nullptr);
    virtual ~RasterizedVolume() {};

    virtual bool isValid() const
    { return m_mesh != nullptr || RawVolume<T>::isValid();}

//    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const;
//
//    virtual itkVolumeType::Pointer toITK();
//
//    virtual const itkVolumeType::Pointer toITK() const;
//
//    virtual vtkAlgorithmOutput *toVTK();
//
//    virtual const vtkAlgorithmOutput *toVTK() const;

    double memoryUsage() const;

    void setOrigin(const typename T::PointType origin);

    typename T::PointType origin() const;

    void setSpacing(const typename T::SpacingType spacing);

    typename T::SpacingType spacing() const;

    const VolumetricData<T>::itkImageSPtr itkImage() const;

    const VolumetricData<T>::itkImageSPtr itkImage(const Bounds& bounds) const;


  private:
    void rasterize(double *bounds) const;
    void transformVTK2ITK() const;
    void updateITKVolume() const;

  private:
    vtkSmartPointer<vtkPolyData> m_mesh;
    itkVolumeType::SpacingType  m_spacing;

    mutable vtkSmartPointer<vtkAlgorithmOutput>          m_vtkVolume;

    mutable vtkSmartPointer<vtkImageData>                m_emptyImage;
    mutable vtkSmartPointer<vtkImplicitPolyDataDistance> m_distance;

    using itkImageImporter = itk::VTKImageImport<T>;

    mutable typename itkImageImporter::Pointer           m_itkImporter;
    mutable vtkSmartPointer<vtkImageExport>              m_vtkExporter;

    mutable unsigned long int m_rasterizationTime;
    mutable double            m_rasterizationBounds[6];
  };

  template<class T> using RasterizedVolumePtr = RasterizedVolume<T> *;
  template<class T> using RasterizedVolumeSPtr = std::shared_ptr<RasterizedVolume<T>>;

  template<class T> RasterizedVolumeSPtr<T> EspinaCore_EXPORT rasterizedVolume(OutputSPtr output);
}

#endif // ESPINA_RASTERIZEDVOLUME_H
