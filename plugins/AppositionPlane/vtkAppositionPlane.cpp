/*
 *    Copyright (c) 2011, Jorge Peña <jorge.pena.pastor@gmail.com>
 *    All rights reserved.
 * 
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 * 
 *    THIS SOFTWARE IS PROVIDED BY Jorge Peña <jorge.pena.pastor@gmail.com> ''AS IS'' AND ANY
 *    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *    DISCLAIMED. IN NO EVENT SHALL Jorge Peña <jorge.pena.pastor@gmail.com> BE LIABLE FOR ANY
 *    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vtkAppositionPlane.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <assert.h>

#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkOBBTree.h>
#include <vtkPlaneSource.h>

#include <vtkMath.h>

#include <vtkGridTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkImplicitVolume.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <itkVTKImageToImageFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkExtractImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkSignedDanielssonDistanceMapImageFilter.h>
#include <itkGradientImageFilter.h>
#include <vtkClipPolyData.h>

typedef unsigned char				     SegPixelType;
typedef itk::Image<SegPixelType, 3> 		     SegImageType;
typedef itk::ImageRegionConstIterator<SegImageType>  SegPixelIterator;

typedef float					     DistanceType;
typedef itk::Image<DistanceType,3>		     DistanceMapType;
typedef itk::ImageRegionConstIterator<DistanceMapType> DistanceIterator;

typedef vtkSmartPointer<vtkPolyData> Plane;

typedef itk::VTKImageToImageFilter<SegImageType>     VtkToItkFilterType;
typedef itk::ImageToVTKImageFilter<SegImageType>     ItkToVtkFilterType;
typedef itk::ExtractImageFilter<SegImageType,
				SegImageType>	     ExtractFilterType;
typedef itk::ConstantPadImageFilter<SegImageType,
				    SegImageType>    PadFilterType;

typedef vtkSmartPointer<vtkPoints>  		     Points;
typedef vtkSmartPointer<vtkOBBTree>		     OBBTreeType;
typedef itk::SignedDanielssonDistanceMapImageFilter
  <SegImageType, DistanceMapType> SDDistanceMapFilterType;
typedef vtkSmartPointer<vtkPlaneSource>		     PlaneSourceType;
typedef itk::GradientImageFilter<DistanceMapType, float>  GradientFilterType;
typedef itk::CovariantVector<float, 3> CovariantVectorType;
typedef itk::Image<CovariantVectorType,3> CovariantVectorImageType;
typedef vtkSmartPointer<vtkGridTransform> GridTransform;
typedef vtkSmartPointer<vtkTransformPolyDataFilter> TransformPolyDataFilter;

//-----------------------------------------------------------------------------
/// Return a cloud of points representing the segmentation
/// Segmentations are represented by labelmap-like vtkDataImages
/// with background pixels being 0 and foreground ones being 255.
/// Nevertheless, non-0 pixels are also considered foreground.
Points segmentationPoints(SegImageType::Pointer seg)
{
  //SegImageType::SpacingType sp = seg->GetSpacing();
  itk::Vector<double, 3> spacing = seg->GetSpacing();
    double spcx, spcy, spcz;                          
    spcx = spacing[0];                                
    spcy = spacing[1];                                
    spcz = spacing[2];     
  
  Points points = Points::New();
  
  SegPixelIterator it(seg, seg->GetLargestPossibleRegion());
  while (!it.IsAtEnd())
  {
    SegPixelType val = it.Get();
    SegImageType::IndexType index = it.GetIndex();
    if (val > 0) 
      points->InsertNextPoint(index[0]*spcx, index[1]*spcy, index[2]*spcz);
    ++it;
  }
  return points;
}

//-----------------------------------------------------------------------------
/// Return the 8 corners of an OBB
Points corners(double corner[3], double max[3], double mid[3], double min[3])
{
  Points points = Points::New();
  double x[3];
  
  // {0,0,0}  <- in a cube
  x[0] = corner[0];
  x[1] = corner[1];
  x[2] = corner[2];
  points->InsertNextPoint(x);
  // {1,0,0}  <- in a cube
  x[0] = corner[0] + mid[0];
  x[1] = corner[1] + mid[1];
  x[2] = corner[2] + mid[2];
  points->InsertNextPoint(x);
  // {0,1,0}  <- in a cube
  x[0] = corner[0] + max[0];
  x[1] = corner[1] + max[1];
  x[2] = corner[2] + max[2];
  points->InsertNextPoint(x);
  // {1,1,0}  <- in a cube
  x[0] = corner[0] + max[0] + mid[0];
  x[1] = corner[1] + max[1] + mid[1];
  x[2] = corner[2] + max[2] + mid[2];
  points->InsertNextPoint(x);
  // {0,0,1}  <- in a cube
  x[0] = corner[0] + min[0];
  x[1] = corner[1] + min[1];
  x[2] = corner[2] + min[2];
  points->InsertNextPoint(x);
  // {1,0,1}  <- in a cube
  x[0] = corner[0] + mid[0] + min[0];
  x[1] = corner[1] + mid[1] + min[1];
  x[2] = corner[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  // {0,1,1}  <- in a cube
  x[0] = corner[0] + max[0] + min[0];
  x[1] = corner[1] + max[1] + min[1];
  x[2] = corner[2] + max[2] + min[2];
  points->InsertNextPoint(x);
  // {1,1,1}  <- in a cube
  x[0] = corner[0] + max[0] + mid[0] + min[0];
  x[1] = corner[1] + max[1] + mid[1] + min[1];
  x[2] = corner[2] + max[2] + mid[2] + min[2];
  points->InsertNextPoint(x);
  
  return points;
}

//-----------------------------------------------------------------------------
DistanceMapType::Pointer computeDistanceMap(SegImageType::Pointer seg)
{
  SDDistanceMapFilterType::Pointer sddm_filter = SDDistanceMapFilterType::New();
  sddm_filter->InsideIsPositiveOn();
  sddm_filter->UseImageSpacingOn();
  sddm_filter->SquaredDistanceOn();
  sddm_filter->SetInput(seg);
  sddm_filter->Update();
  
  return sddm_filter->GetOutput();
}

//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void maxDistancePoint(DistanceMapType::Pointer map, Points points, double avgMaxDistPoint[3])
{
  DistanceType maxDist = 0;
  DistanceMapType::SpacingType spacing = map->GetSpacing();
  
  DistanceIterator it(map, map->GetLargestPossibleRegion());
  while (!it.IsAtEnd())
  {
    DistanceType dist = it.Get();                                           
    if (dist > maxDist) {                                                            
      DistanceMapType::IndexType index = it.GetIndex();                     
      maxDist = dist; 
      for (unsigned int i = 0; i < 3; i++)
	avgMaxDistPoint[i] = index[i] * spacing[i];
      
      points->Initialize();
      points->InsertNextPoint(avgMaxDistPoint);                                           
    }   
    else if (dist == maxDist) {
      DistanceMapType::IndexType index = it.GetIndex();                     
      for (unsigned int i = 0; i < 3; i++)
	avgMaxDistPoint[i] += index[i] * spacing[i];
      
      points->InsertNextPoint(index[0]*spacing[0], index[1]*spacing[1], index[2]*spacing[2]);
    }                                                                                 
    ++it;
  }
  
  for (unsigned int i = 0; i < 3; i++)
    avgMaxDistPoint[i] /= points->GetNumberOfPoints();                                      
}

//-----------------------------------------------------------------------------
Plane clipPlane(Plane plane, vtkImageData* image)
{
    vtkSmartPointer<vtkImplicitVolume> implicVolFilter =
            vtkSmartPointer<vtkImplicitVolume>::New();
    implicVolFilter->SetVolume(image);
    implicVolFilter->SetOutValue(0);
    //vtk_image->Print(std::cout);

    vtkSmartPointer<vtkClipPolyData> clipper =
            vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetClipFunction(implicVolFilter);
    clipper->SetInput(plane);
    clipper->SetValue(0);
    clipper->Update();

    Plane clippedPlane = Plane::New();
    clippedPlane->DeepCopy(clipper->GetOutput());

    return clippedPlane;
}

//-----------------------------------------------------------------------------
/// Find the projection of A on B
void project(const double A[3], const double B[3], double Projection[3])
{
    double scale = vtkMath::Dot(A,B)/pow(vtkMath::Norm(B), 2);
    for(unsigned int i = 0; i < 3; i++)
        Projection[i] = scale * B[i];
}

//-----------------------------------------------------------------------------
void vectorImageToVTKImage(CovariantVectorImageType::Pointer vectorImage, vtkImageData* image)
{
  CovariantVectorImageType::RegionType region = vectorImage->GetLargestPossibleRegion();
  CovariantVectorImageType::SizeType imageSize = region.GetSize();
  image->SetExtent(0, imageSize[0] -1, 0, imageSize[1] - 1, 0, imageSize[2] - 1);
  CovariantVectorImageType::SpacingType spacing = vectorImage->GetSpacing();
  image->SetSpacing(spacing[0], spacing[1], spacing[2]);
//   image->SetSpacing(vectorImage->GetSpacing()[0], vectorImage->GetSpacing()[1], vectorImage->GetSpacing()[2]);

  vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(imageSize[0] * imageSize[1] * imageSize[2]);
  vectors->SetName("GradientVectors");

  int counter = 0;
  for(unsigned int k = 0; k < imageSize[2]; k++)
    for(unsigned int j = 0; j < imageSize[1]; j++)
      for(unsigned int i = 0; i < imageSize[0]; i++)
      {
	CovariantVectorImageType::IndexType index;
	index[0] = i;
	index[1] = j;
	index[2] = k;
	
	CovariantVectorType pixel = vectorImage->GetPixel(index);
	float val[3];
	val[0] = pixel[0];
	val[1] = pixel[1];
	val[2] = pixel[2];
	
	vectors->InsertTupleValue(counter, val);
	counter++;
      }
  //std::cout << region << std::endl;

  image->GetPointData()->SetVectors(vectors);
  image->GetPointData()->SetScalars(vectors);
}

//-----------------------------------------------------------------------------
void projectVectors(vtkImageData * vectors_image, double * unitary)
{
  vtkSmartPointer<vtkDataArray> vectors = vectors_image->GetPointData()->GetVectors();
  int number_of_tuples = vectors->GetNumberOfTuples();
  double projv[3];
  for (int i = 0; i < number_of_tuples; i++) {
    double * v = vectors->GetTuple(i);
    project(v, unitary, projv);
    vectors->SetTuple(i, projv);
  }
}


//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAppositionPlaneFilter);

//-----------------------------------------------------------------------------
vtkAppositionPlaneFilter::vtkAppositionPlaneFilter()
: Resolution(50)
, NumIterations(5)
, Converge(true)
{
  bzero(Inclusion,3*sizeof(int));
  bzero(Exclusion,3*sizeof(int));
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkAppositionPlaneFilter::~vtkAppositionPlaneFilter()
{
  
}

//-----------------------------------------------------------------------------
int vtkAppositionPlaneFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAppositionPlaneFilter::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

//-----------------------------------------------------------------------------
int vtkAppositionPlaneFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *imageInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *appPlaneInfo = outputVector->GetInformationObject(0);
  
  // Access to the input image (stack)
  vtkImageData *image = vtkImageData::SafeDownCast(
    imageInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  vtkPolyData* plane = vtkPolyData::SafeDownCast(
    appPlaneInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  
  double origin[3];
  image->GetOrigin(origin);
  vtkDebugMacro( << "Origin " << origin);
  
  vtkDebugMacro( << "Convet from VTK to ITK");
  
  VtkToItkFilterType::Pointer vtk2itk_filter = VtkToItkFilterType::New();
  vtk2itk_filter->SetInput(image);
  vtk2itk_filter->Update();
  
  vtkDebugMacro( << "Padding Image");
  
  SegImageType::SizeType bounds;
  bounds[0] = bounds[1] = bounds[2] = 1;
  PadFilterType::Pointer padder = PadFilterType::New();
  padder->SetInput(vtk2itk_filter->GetOutput());
  padder->SetPadLowerBound(bounds);
  padder->SetPadUpperBound(bounds);
  padder->SetConstant(0); // extend with black pixels
  padder->Update();
  SegImageType::Pointer padImage = padder->GetOutput();
  
  SegImageType::RegionType  region = padImage->GetLargestPossibleRegion();
  SegImageType::SizeType imageSize = region.GetSize();
  region.SetIndex(region.GetIndex() + bounds);
  padImage->SetRegions(region);
  
  
  vtkDebugMacro( << "Compute Distamce Map");
  
  Points points = segmentationPoints(padImage);
  OBBTreeType obbTree = OBBTreeType::New();
  
  double corner[3], max[3], mid[3], min[3], size[3];
  obbTree->ComputeOBB(points, corner, max, mid, min, size);
  Points obbCorners = corners(corner, max, mid, min);
  DistanceMapType::Pointer distanceMap = computeDistanceMap(padImage);

  
  vtkDebugMacro( << "Build and move the plane to Avg Max Distance");
  
  Points maxPoints = Points::New();
  double avgMaxDistPoint[3];
  maxDistancePoint(distanceMap, maxPoints, avgMaxDistPoint);
  PlaneSourceType planeSource = PlaneSourceType::New();
  planeSource->SetOrigin(obbCorners->GetPoint(0));
  planeSource->SetPoint1(obbCorners->GetPoint(1));
  planeSource->SetPoint2(obbCorners->GetPoint(2));
  planeSource->SetResolution(Resolution, Resolution);
  planeSource->Update();
  
  
  vtkDebugMacro( << "Create Path with point + min and update min\n"
		    "Fill vtkthinPlatesplineTransform");
  double normal[3];
  planeSource->GetNormal(normal);
  vtkMath::Normalize(normal);
  
  double v[3], displacement[3];
  for (int i = 0; i < 3; i++) {
    v[i] = avgMaxDistPoint[i] - obbCorners->GetPoint(0)[i];
  }
  
  project(v, normal, displacement);
  int sign = vtkMath::Dot(displacement, normal) > 0?1:-1;
  planeSource->Push( sign * vtkMath::Norm(displacement));
  planeSource->Update();
  Plane sourcePlane = planeSource->GetOutput();

  // Plane is only transformed in its normal direction
  vtkDebugMacro( << "Compute transformation matrix from distance map gradient");
  GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetInput(distanceMap);
  gradientFilter->Update();
  
  vtkSmartPointer<vtkImageData> gradientVectorGrid =
  vtkSmartPointer<vtkImageData>::New();
  vectorImageToVTKImage(gradientFilter->GetOutput(), gradientVectorGrid);
  //gradientVectorGrid->Print(std::cout);
  
  
  projectVectors(gradientVectorGrid, normal);
  
  GridTransform grid_transform = GridTransform::New();
  grid_transform->SetDisplacementGrid(gradientVectorGrid);
  grid_transform->SetInterpolationModeToCubic();
  
  TransformPolyDataFilter transformer = TransformPolyDataFilter::New();
  Plane auxPlane = sourcePlane;

  if (Converge) {
    double *spacing = image->GetSpacing();
    double min_in_pixels[3] = {0,0,0};
    for (unsigned int i=0; i < 3; i++) {
      min_in_pixels[i] = min[i] / spacing[i];
    }   
    NumIterations = std::max( 1, int(floor(sqrt(vtkMath::Norm(min_in_pixels)))));
  }   
  
  std::cerr << "Number of iterations: " << NumIterations << std::endl;
  
  transformer->SetTransform(grid_transform);
  for (int i =0; i <= NumIterations; i++) {
    transformer->SetInput(auxPlane);
    transformer->Modified();
    transformer->Update();
    
    auxPlane->DeepCopy(transformer->GetOutput());
  }
  
  Plane clippedPlane = clipPlane(transformer->GetOutput(), image);

  vtkDebugMacro( << "Correct Plane's visualization and cell area's computation");
  vtkSmartPointer<vtkTriangleFilter> triangle_filter =
  vtkSmartPointer<vtkTriangleFilter>::New();
  triangle_filter->SetInput(clippedPlane);
  triangle_filter->Update();
  
  vtkSmartPointer<vtkPolyDataNormals> normals =
  vtkSmartPointer<vtkPolyDataNormals>::New();
  normals->SetInput(triangle_filter->GetOutput());
  normals->SplittingOff();
  normals->Update();
  
  vtkSmartPointer<vtkPolyData> appositionPlane = normals->GetOutput();
  
  
  vtkDebugMacro( << "Create Mesh");
  plane->DeepCopy(appositionPlane);
  
  return 1;
}