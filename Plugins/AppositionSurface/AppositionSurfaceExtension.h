/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef APPOSITIONSURFACEXTENSION_H
#define APPOSITIONSURFACEXTENSION_H

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/EspinaTypes.h>

// ITK
#include <itkConstantPadImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkSignedDanielssonDistanceMapImageFilter.h>
#include <itkImageToVTKImageFilter.h>

// VTK
#include <vtkGridTransform.h>
#include <vtkOBBTree.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>

class vtkImageData;
class AppositionSurfaceExtension
: public SegmentationExtension
{
  typedef float DistanceType;
  typedef vtkSmartPointer<vtkPoints>   Points;
  typedef vtkSmartPointer<vtkPolyData> PolyData;
  typedef vtkSmartPointer<vtkOBBTree>  OBBTreeType;

  typedef itk::ImageRegionConstIterator<itkVolumeType>  VoxelIterator;
  typedef itk::ImageToVTKImageFilter<itkVolumeType> ItkToVtkFilterType;
  typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType> ExtractFilterType;
  typedef itk::ConstantPadImageFilter<itkVolumeType, itkVolumeType> PadFilterType;
  typedef itk::Image<DistanceType,3> DistanceMapType;
  typedef itk::ImageRegionConstIterator<DistanceMapType> DistanceIterator;
  typedef itk::SignedDanielssonDistanceMapImageFilter
  <itkVolumeType, DistanceMapType>  SDDistanceMapFilterType;
  typedef vtkSmartPointer<vtkPlaneSource> PlaneSourceType;
  typedef itk::GradientImageFilter<DistanceMapType, float> GradientFilterType;
  typedef vtkSmartPointer<vtkGridTransform> GridTransform;
  typedef vtkSmartPointer<vtkTransformPolyDataFilter>  TransformPolyDataFilter;
  typedef itk::CovariantVector<float, 3> CovariantVectorType;
  typedef itk::Image<CovariantVectorType,3> CovariantVectorImageType;

public:
  static const ExtId ID;
  static const InfoTag AREA;
  static const InfoTag PERIMETER;

public:
  explicit AppositionSurfaceExtension();
  virtual ~AppositionSurfaceExtension();

  virtual ExtId id();

  virtual ExtIdList dependencies() const
  { return SegmentationExtension::dependencies(); }

  virtual InfoList availableInformations() const
  { return SegmentationExtension::availableInformations(); }

  virtual RepList availableRepresentations() const
  { return SegmentationExtension::availableRepresentations(); }

  virtual QVariant information(InfoTag tag) const;

  virtual SegmentationRepresentation* representation(QString rep);

  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

  virtual SegmentationExtension* clone();

  PolyData appositionSurface() const
  { return m_ap; }

  //NOTE: Constness is required by information call
  bool updateAppositionSurface() const;

private:
  // Apposition Plane Auxiliar Functions
  PolyData clipPlane(AppositionSurfaceExtension::PolyData plane, vtkImageData* image) const;
  DistanceMapType::Pointer computeDistanceMap(itkVolumeType::Pointer volume) const;
  /// Return the 8 corners of an OBB
  Points corners(double corner[3], double max[3], double mid[3], double min[3]) const;
  void maxDistancePoint(DistanceMapType::Pointer map, Points points, double avgMaxDistPoint[3]) const;
  /// Return a cloud of points representing the segmentation
  /// Segmentations are represented by labelmap-like vtkDataImages
  /// with background pixels being 0 and foreground ones being 255.
  /// Nevertheless, non-0 pixels are also considered foreground.
  Points segmentationPoints(itkVolumeType::Pointer seg) const;
  /// Find the projection of A on B
  void project(const double *A, const double *B, double *Projection) const;
  void projectVectors(vtkImageData * vectors_image, double * unitary) const;
  void vectorImageToVTKImage(CovariantVectorImageType::Pointer vectorImage, vtkImageData *image) const;

  // Apposition Plane Metrics
  double computeArea() const;
  double computePerimeter() const;
  bool isPerimeter(vtkIdType cellId, vtkIdType p1, vtkIdType p2) const;

private:
  int      m_resolution;
  int      m_iterations;
  bool     m_converge;

  mutable double   m_area;
  mutable double   m_perimeter;
  mutable PolyData m_ap;

  mutable itk::TimeStamp m_lastUpdate;
};

#endif // APPOSITIONSURFACEEXTENSION_H
