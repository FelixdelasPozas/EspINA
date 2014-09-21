/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef APPOSITIONSURFACEFILTER_H_
#define APPOSITIONSURFACEFILTER_H_

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>

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

class QString;

namespace ESPINA
{
  class SASFetchBehaviour;

  const Filter::Type AS_FILTER = "AppositionSurface::AppositionSurfaceFilter";

  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilter
  : public Filter
  {
    Q_OBJECT
    static constexpr double   THRESHOLDFACTOR           = 0.005; // Percentage of a single step
    static const unsigned int MAXSAVEDSTATUSES          = 10;
    static const int          MAXITERATIONSFACTOR       = 100;
    static constexpr float    DISPLACEMENTSCALE         = 1;
    static constexpr float    CLIPPINGTHRESHOLD         = 0.5;
    static constexpr float    DISTANCESMOOTHSIGMAFACTOR = 0.67448; // probit(0.25)

    using DistanceType = float;
    using Points = vtkSmartPointer<vtkPoints>;
    using PolyData = vtkSmartPointer<vtkPolyData>;
    using OBBTreeType = vtkSmartPointer<vtkOBBTree>;

    using itkVolumeIterator = itk::ImageRegionIterator<itkVolumeType>;
    using ItkToVtkFilterType = itk::ImageToVTKImageFilter<itkVolumeType>;
    using ExtractFilterType = itk::ExtractImageFilter<itkVolumeType, itkVolumeType>;
    using PadFilterType = itk::ConstantPadImageFilter<itkVolumeType, itkVolumeType>;

    using GridTransform = vtkSmartPointer<vtkGridTransform>;
    using TransformPolyDataFilter = vtkSmartPointer<vtkTransformPolyDataFilter>;

    using DistanceMapType = itk::Image<DistanceType,3>;
    using DistanceIterator = itk::ImageRegionConstIterator<DistanceMapType>;
    using SMDistanceMapFilterType = itk::SignedMaurerDistanceMapImageFilter<itkVolumeType, DistanceMapType>;
    using SmoothingFilterType = itk::SmoothingRecursiveGaussianImageFilter<DistanceMapType, DistanceMapType>;
    using GradientFilterType = itk::GradientImageFilter<DistanceMapType, float>;

    using PlaneSourceType = vtkSmartPointer<vtkPlaneSource>;
    using CovariantVectorType = itk::CovariantVector<float, 3>;
    using CovariantVectorImageType = itk::Image<CovariantVectorType,3>;
    using PointsListType = std::list< vtkSmartPointer<vtkPoints> >;

  public:

    static const char * MESH_ORIGIN;
    static const char * MESH_NORMAL;

  public:
    explicit AppositionSurfaceFilter(InputSList inputs, Type type, SchedulerSPtr scheduler);
    virtual ~AppositionSurfaceFilter();

  protected:

    /** brief Implements Persistent::restoreState().
     *
     */
    virtual void restoreState(const State& state)
    {};

    /** brief Implements Persistent::state().
     *
     */
    virtual State state() const
    { return State(); }

    /** brief Implements Filter::safeFilterSnapshot().
     *
     */
    virtual Snapshot saveFilterSnapshot() const
    { return Snapshot(); }

    /** brief Implements Filter::needUpdate().
     *
     */
    virtual bool needUpdate() const;

    /** brief Implements Filter::needUpdate(oid).
     *
     */
    virtual bool needUpdate(Output::Id oid) const;

    /** brief Implements Filter::execute().
     *
     */
    virtual void execute();

    /** \brief Imlements Filter::execute(oid).
     *
     */
    virtual void execute(Output::Id id);

    /** brief Implements Filter::ignoreStorageContent()
     *
     */
    virtual bool ignoreStorageContent() const
    { return this->m_alreadyFetchedData; }

    /** brief Implements Filter::invalidateEditedRegions().
     *
     */
    virtual bool invalidateEditedRegions()
    { return false; }

  protected slots:
    virtual void inputModified();

  private:
    /** brief Returns a cloud of points representing the segmentation.
     * Segmentations are represented by labelmap-like vtkDataImages
     * with background pixels being 0 and foreground ones being 255.
     * Nevertheless, non-0 pixels are also considered foreground.
     * \param[in] seg, reference to a itk::image<unsigned char, 3>
     *
     */
    Points segmentationPoints(const itkVolumeType::Pointer &seg) const;

    /** brief Returns the 8 corners of an OBB.
     * \param[in] corner
     * \param[in] max
     * \param[in] mid
     * \param[in] min
     */
    Points corners(const double corner[3], const double max[3], const double mid[3], const double min[3]) const;

    /** brief Returns a distance map of the volume passed as parameter.
     * \param[in] volume, itk::Image<unsigned char, 3>::Pointer.
     * \param[in] sigma
     */
    DistanceMapType::Pointer computeDistanceMap(const itkVolumeType::Pointer &volume, const float sigma) const;

    void maxDistancePoint(const DistanceMapType::Pointer &map, double avgMaxDistPoint[3], double & maxDist) const;
    void computeResolution(const double *max, const double *mid, const double *spacing, int & xResolution, int & yResolution) const;
    void computeIterationLimits(const double *min, const double *spacing, int & iterations, double & thresholdError) const;

    /** brief Find the projection of A on B.
     *
     */
    void project(const double *A, const double *B, double *Projection) const;
    void projectVectors(vtkImageData* vectors_image, double *unitary) const;

    void vectorImageToVTKImage(const CovariantVectorImageType::Pointer vectorImage, vtkSmartPointer<vtkImageData> image) const;

    bool hasConverged(vtkPoints * lastPlanePoints, PointsListType & pointsList, double threshold) const;
    int computeMeanEuclideanError(vtkPoints * pointsA, vtkPoints * pointsB, double & euclideanError) const;
    PolyData clipPlane(vtkPolyData *plane, vtkImageData* image) const;
    PolyData triangulate(PolyData plane) const;

  private:
    int m_resolution;
    int m_iterations;
    bool m_converge;
    mutable PolyData m_ap;

    itkVolumeType::Pointer m_input;

    bool m_alreadyFetchedData;
    TimeStamp m_lastModifiedMesh;

    friend class SASFetchBehaviour;
  };

  using AppositionSurfaceFilterPtr  = AppositionSurfaceFilter *;
  using AppositionSurfaceFilterSPtr = std::shared_ptr<AppositionSurfaceFilter>;

} /* namespace ESPINA */
#endif /* APPOSITIONSURFACEFILTER_H_ */
