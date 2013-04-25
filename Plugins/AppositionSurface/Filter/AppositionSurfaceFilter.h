/*
 * AppositionSurfaceFilter.h
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACEFILTER_H_
#define APPOSITIONSURFACEFILTER_H_

// EspINA
#include <Core/Model/Filter.h>
#include <Core/Model/ModelItem.h>

// ITK
#include <itkConstantPadImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkSmoothingRecursiveGaussianImageFilter.h>

// VTK
#include <vtkGridTransform.h>
#include <vtkOBBTree.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>

// STL
#include <list>

// Qt
class QString;

namespace EspINA
{
  class AppositionSurfaceVolume;

  class AppositionSurfaceFilter
  : public SegmentationFilter
  {
    Q_OBJECT
      static const double THRESHOLDFACTOR = 0.01; // Percentage of a single step
      static const unsigned int MAXSAVEDSTATUSES = 10;
      static const int MAXITERATIONSFACTOR = 100;
      static const float DISPLACEMENTSCALE = 1;
      static const float CLIPPINGTHRESHOLD = 0.5;
      static const float DISTANCESMOOTHSIGMAFACTOR = 0.67448; // probit(0.25)

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
      typedef itk::SignedMaurerDistanceMapImageFilter
      <itkVolumeType, DistanceMapType>  SMDistanceMapFilterType;
      typedef itk::SmoothingRecursiveGaussianImageFilter
      <DistanceMapType, DistanceMapType>  SmoothingFilterType;
      typedef vtkSmartPointer<vtkPlaneSource> PlaneSourceType;
      typedef itk::GradientImageFilter<DistanceMapType, float> GradientFilterType;
      typedef vtkSmartPointer<vtkGridTransform> GridTransform;
      typedef vtkSmartPointer<vtkTransformPolyDataFilter>  TransformPolyDataFilter;
      typedef itk::CovariantVector<float, 3> CovariantVectorType;
      typedef itk::Image<CovariantVectorType,3> CovariantVectorImageType;

      typedef std::list< vtkSmartPointer<vtkPoints> > PointsListType;

    public:
      static const QString INPUTLINK;
      static const QString SAS;

      static const ArgumentId ORIGIN;

      typedef AppositionSurfaceFilter *               Pointer;
      typedef QSharedPointer<AppositionSurfaceFilter> SPointer;

    public:
      AppositionSurfaceFilter(NamedInputs inputs,
                              Arguments   args,
                              FilterType  filter);
      virtual ~AppositionSurfaceFilter();

      virtual void upkeeping();

      virtual QString getOriginSegmentation();
      virtual itkVolumeType::SpacingType getOriginSpacing();
      virtual itkVolumeType::RegionType getOriginRegion();
      virtual double getArea();
      virtual double getPerimeter();
      virtual double getTortuosity();

      virtual bool dumpSnapshot(Snapshot &snapshot);

      /// Try to locate an snapshot of the filter in tmpDir
      /// Returns true if all volume snapshot can be recovered
      /// and false otherwise
      virtual bool fetchCachePolyDatas();

    protected slots:
      virtual void inputModified();

    protected:
      /// Wether filter parameters have changed or not
      virtual bool ignoreCurrentOutputs() const
      { return false; }

      /// Whether the filter needs to be updated or not
      /// Default implementation will request an update if there is no filter output
      /// or it is an invalid output
      virtual bool needUpdate(FilterOutputId oId) const;

      virtual void run();

      virtual void run(FilterOutputId oId);

      virtual void createOutput(FilterOutputId id, itkVolumeType::Pointer volume = NULL);

    private:
      /// forbidden methods
      /////////////////////////////////////////////////////////////////////
      virtual void draw(FilterOutputId oId,
                        vtkImplicitFunction *brush,
                        const Nm bounds[6],
                        itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                        bool emitSignal = true) {};

      virtual void draw(FilterOutputId oId,
                        itkVolumeType::IndexType index,
                        itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                        bool emitSignal = true) {};

      virtual void draw(FilterOutputId oId,
                        Nm x, Nm y, Nm z,
                        itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                        bool emitSignal = true) {};
      virtual void draw(FilterOutputId oId,
                        vtkPolyData *contour,
                        Nm slice,
                        PlaneType plane,
                        itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                        bool emitSignal = true) {};

      virtual void draw(FilterOutputId oId,
                        itkVolumeType::Pointer volume,
                        bool emitSignal = true)  {};

      //TODO 2012-11-20 cambiar nombre y usar FilterOutput
      virtual void restoreOutput(FilterOutputId oId,
                                 itkVolumeType::Pointer volume) {};

      /// helper methods
      /////////////////////////////////////////////////////////////////////

      /// Return a cloud of points representing the segmentation
      /// Segmentations are represented by labelmap-like vtkDataImages
      /// with background pixels being 0 and foreground ones being 255.
      /// Nevertheless, non-0 pixels are also considered foreground.
      Points segmentationPoints(itkVolumeType::Pointer seg) const;

      /// Return the 8 corners of an OBB
      Points corners(double corner[3], double max[3], double mid[3], double min[3]) const;

      DistanceMapType::Pointer computeDistanceMap(itkVolumeType::Pointer volume, float sigma) const;
      void maxDistancePoint(DistanceMapType::Pointer map, double avgMaxDistPoint[3], double & maxDist) const;
      void computeResolution(double * max, double * mid, double * spacing, int & xResolution, int & yResolution) const;
      void computeIterationLimits(double * min, double * spacing, int & iterations, double & thresholdError) const;

      /// Find the projection of A on B
      void project(const double* A, const double* B, double* Projection) const;
      void projectVectors(vtkImageData* vectors_image, double* unitary) const;
      void projectPolyDataToPlane(double* origin, double* normal, vtkPolyData* meshIn, vtkPolyData* meshOut) const;

      void vectorImageToVTKImage(CovariantVectorImageType::Pointer vectorImage, vtkSmartPointer<vtkImageData> image) const;

      bool hasConverged( vtkPoints * lastPlanePoints, PointsListType & pointsList, double threshold) const;
      int computeMeanEuclideanError(vtkPoints * pointsA, vtkPoints * pointsB, double & euclideanError) const;
      PolyData clipPlane(PolyData plane, vtkImageData* image) const;
      PolyData triangulate(PolyData plane) const;
      /////////////////////////////////////////////////////////////////////

      /// Apposition plane metrics
      /////////////////////////////////////////////////////////////////////
      double computeArea() const;
      double computeArea(PolyData mesh) const;
      double computePerimeter() const;
      double computeTortuosity() const;
      bool isPerimeter(vtkIdType cellId, vtkIdType p1, vtkIdType p2) const;
      /////////////////////////////////////////////////////////////////////

      int m_resolution;
      int m_iterations;
      bool m_converge;
      mutable PolyData m_ap;
      mutable double*  m_templateOrigin;
      mutable double*  m_templateNormal;
      mutable SegmentationPtr m_originSegmentation;

      QString m_origin;
      mutable double m_area;
      mutable double m_perimeter;
      mutable double m_tortuosity;

      itkVolumeType::Pointer m_input;

      bool m_alreadyFetchedData;

      friend class AppositionSurfaceVolume;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACEFILTER_H_ */
