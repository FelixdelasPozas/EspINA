/*
 * vtkTubeSource.h
 *
 *  Created on: August 2012
 *      Author: Félix de las Pozas Álvarez
 */

// .NAME vtkTubeSource - generate a tube centered at origin
// .SECTION Description
// vtkTubeSource creates a polygonal cylinder centered at Center;
// The axis of the tube is aligned along the global y-axis.
// The height, top and bottom radius of the tube can be specified, as
// well as the number of sides. It is also possible to control whether
// the tube is open-ended or capped.

#ifndef _VTKTUBESOURCE_H_
#define _VTKTUBESOURCE_H_

#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkCell.h>

class vtkTransform;
class vtkTransformPolyDataFilter;

class VTK_GRAPHICS_EXPORT vtkTubeSource : public vtkPolyDataAlgorithm
{
public:
  static vtkTubeSource *New();
  vtkTypeMacro(vtkTubeSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the radius of the bottom of the tube. Initial value is 0.5
  vtkSetClampMacro(BottomRadius,double,0.0,VTK_DOUBLE_MAX)
  vtkGetMacro(BottomRadius,double);

  // Description:
  // Set the radius of the top of the tube. Initial value is 0.5
  vtkSetClampMacro(TopRadius,double,0.0,VTK_DOUBLE_MAX)
  vtkGetMacro(TopRadius,double);

  // Description:
  // Set/Get bottom center. Initial value is (0.0,0.0,0.0)
  vtkSetVector3Macro(BottomCenter,double);
  vtkGetVectorMacro(BottomCenter,double,3);

  // Description:
  // Set/Get top center. Initial value is (0.0,0.0,0.0)
  vtkSetVector3Macro(TopCenter,double);
  vtkGetVectorMacro(TopCenter,double,3);

  // Description:
  // Set the number of facets used to define cylinder. Initial value is 50.
  vtkSetClampMacro(Resolution,int,2,VTK_CELL_SIZE)
  vtkGetMacro(Resolution,int);

  // Description:
  // Turn on/off whether to cap cylinder with polygons. Initial value is false.
  vtkSetMacro(Capping,int);
  vtkGetMacro(Capping,int);
  vtkBooleanMacro(Capping,int);

protected:
  vtkTubeSource(int res=50);
  ~vtkTubeSource() {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  double BottomRadius;
  double TopRadius;
  double BottomCenter[3];
  double TopCenter[3];
  int Resolution;
  int Capping;

private:
  vtkTubeSource(const vtkTubeSource&);  // Not implemented.
  void operator=(const vtkTubeSource&);  // Not implemented.

  vtkSmartPointer<vtkTransform> transform;
  vtkSmartPointer<vtkTransformPolyDataFilter> pdTransform;
};

#endif // _VTKTUBESOURCE_H_
