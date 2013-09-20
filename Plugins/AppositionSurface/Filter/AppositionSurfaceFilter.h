/*
 * AppositionSurfaceFilter.h
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACEFILTER_H_
#define APPOSITIONSURFACEFILTER_H_

#include "AppositionSurfacePlugin_Export.h"

// EspINA
#include <Core/Filters/BasicSegmentationFilter.h>
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
  class RasterizedVolume;

  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilter
  : public BasicSegmentationFilter
  {
    Q_OBJECT
    static const double       THRESHOLDFACTOR;
    static const unsigned int MAXSAVEDSTATUSES;
    static const int          MAXITERATIONSFACTOR;
    static const float        DISPLACEMENTSCALE;
    static const float        CLIPPINGTHRESHOLD;
    static const float        DISTANCESMOOTHSIGMAFACTOR;

    typedef float DistanceType;
    typedef vtkSmartPointer<vtkPoints>   Points;
    typedef vtkSmartPointer<vtkPolyData> PolyData;
    typedef vtkSmartPointer<vtkOBBTree>  OBBTreeType;

    typedef itk::ImageToVTKImageFilter<itkVolumeType>                 ItkToVtkFilterType;
    typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType>     ExtractFilterType;
    typedef itk::ConstantPadImageFilter<itkVolumeType, itkVolumeType> PadFilterType;

    typedef vtkSmartPointer<vtkGridTransform>            GridTransform;
    typedef vtkSmartPointer<vtkTransformPolyDataFilter>  TransformPolyDataFilter;

    typedef itk::Image<DistanceType,3>                       DistanceMapType;
    typedef itk::ImageRegionConstIterator<DistanceMapType>   DistanceIterator;
    typedef itk::SignedMaurerDistanceMapImageFilter
    <itkVolumeType, DistanceMapType>    SMDistanceMapFilterType;
    typedef itk::SmoothingRecursiveGaussianImageFilter
    <DistanceMapType, DistanceMapType>  SmoothingFilterType;
    typedef itk::GradientImageFilter<DistanceMapType, float> GradientFilterType;

    typedef vtkSmartPointer<vtkPlaneSource>                  PlaneSourceType;

    typedef itk::CovariantVector<float, 3>          CovariantVectorType;
    typedef itk::Image<CovariantVectorType,3>       CovariantVectorImageType;
    typedef std::list< vtkSmartPointer<vtkPoints> > PointsListType;

  public:
    static const QString INPUTLINK;
    static const QString SAS;

    static const char * MESH_ORIGIN;
    static const char * MESH_NORMAL;

    static const ArgumentId ORIGIN;

    typedef AppositionSurfaceFilter *                  Pointer;
    typedef boost::shared_ptr<AppositionSurfaceFilter> SPointer;

  public:
    AppositionSurfaceFilter(NamedInputs inputs,
                            Arguments   args,
                            FilterType  filter);
    virtual ~AppositionSurfaceFilter();

    virtual void upkeeping();

    QString getOriginSegmentation();

  protected:
    virtual SegmentationRepresentationSPtr createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type);

    virtual bool ignoreCurrentOutputs() const
    { return false; }

    virtual bool needUpdate() const;

    virtual bool needUpdate(FilterOutputId oId) const;

    virtual void run();

    virtual void run(FilterOutputId oId);

  protected slots:
    virtual void inputModified();

  private:
    virtual itkVolumeType::SpacingType getOriginSpacing();

    virtual itkVolumeType::RegionType getOriginRegion();

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

    void vectorImageToVTKImage(CovariantVectorImageType::Pointer vectorImage, vtkSmartPointer<vtkImageData> image) const;

    bool hasConverged( vtkPoints * lastPlanePoints, PointsListType & pointsList, double threshold) const;
    int computeMeanEuclideanError(vtkPoints * pointsA, vtkPoints * pointsB, double & euclideanError) const;
    PolyData clipPlane(PolyData plane, vtkImageData* image) const;
    PolyData triangulate(PolyData plane) const;
    /////////////////////////////////////////////////////////////////////

  private:
    int m_resolution;
    int m_iterations;
    bool m_converge;
    mutable PolyData m_ap;
    mutable SegmentationPtr m_originSegmentation;

    QString m_origin;

    itkVolumeType::Pointer m_input;

    bool m_alreadyFetchedData;

    friend class AppositionSurfaceVolume;
  };
} /* namespace EspINA */
#endif /* APPOSITIONSURFACEFILTER_H_ */
