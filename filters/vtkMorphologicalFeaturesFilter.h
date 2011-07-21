/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
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


#ifndef VTKMORPHOLOGICALFEATURESFILTER_H
#define VTKMORPHOLOGICALFEATURESFILTER_H

#include <vtkImageAlgorithm.h>

// ITK
#include <itkVTKImageToImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

class VTK_IMAGING_EXPORT vtkMorphologicalFeaturesFilter :
public vtkImageAlgorithm
{
  //BTX
  typedef unsigned char InputPixelType;
  typedef itk::Image<InputPixelType,3> InputImageType;
  // Convert label image to label map
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<InputImageType, LabelMapType> Image2LabelFilterType;
  //ETX
  
public:
  static vtkMorphologicalFeaturesFilter *New();
  vtkTypeMacro(vtkMorphologicalFeaturesFilter, vtkImageAlgorithm);
  
  
  // Morphological Features
  vtkGetMacro(Size, double); //Should be unsigned long
  vtkGetMacro(PhysicalSize, double);
  vtkGetVector3Macro(Centroid, double);
  vtkGetVector3Macro(Region, int);
  vtkGetVector3Macro(BinaryPrincipalMoments, double);
  vtkGetVectorMacro(BinaryPrincipalAxes, double, 9);
  vtkGetMacro(FeretDiameter,double);
  vtkGetVector3Macro(EquivalentEllipsoidSize, double);
  
protected:
  vtkMorphologicalFeaturesFilter();
  virtual ~vtkMorphologicalFeaturesFilter(){}

  virtual int RequestInformation(vtkInformation* request,
				 vtkInformationVector** inputVector,
				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
  
private:
  vtkMorphologicalFeaturesFilter(const vtkImageAlgorithm&);//Not implemented
  void operator=(const vtkImageAlgorithm&);//Not implemented
  
  bool ComputeFeret;
  double Size;
  double PhysicalSize;
  double Centroid[3];
  int Region[3];
  double BinaryPrincipalMoments[3];
  double BinaryPrincipalAxes[9];
  double FeretDiameter;
  double EquivalentEllipsoidSize[3];
 
  //BTX
  Image2LabelFilterType::Pointer image2label;
  //ETX
};

#endif // VTKMORPHOLOGICALFEATURESFILTER_H
