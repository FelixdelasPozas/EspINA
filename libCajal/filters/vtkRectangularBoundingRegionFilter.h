/*
    Copyright (c) 2011, Jorge Peña <jorge.pena.pastor@gmail.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña <jorge.pena.pastor@gmail.com> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña <jorge.pena.pastor@gmail.com> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef VTKRECTANGULARBOUNDINGREGIONFILTER_H
#define VTKRECTANGULARBOUNDINGREGIONFILTER_H

#include <vtkPolyDataAlgorithm.h>


class VTK_IMAGING_EXPORT vtkRectangularBoundingRegionFilter
      : public vtkPolyDataAlgorithm
{
public:
  static vtkRectangularBoundingRegionFilter *New();
  vtkTypeMacro(vtkRectangularBoundingRegionFilter, vtkPolyDataAlgorithm);
  
  vtkSetVector6Macro(Extent,int);
  vtkSetVector3Macro(Spacing,double);

  //! Inclusion Coordinates (Left, Top, Upper)
  vtkSetVector3Macro(Inclusion,int);
  vtkGetVector3Macro(Inclusion,int);
  //! Exclusion Coordinates (Right, Bottom, Lower)
  vtkSetVector3Macro(Exclusion,int);
  vtkGetVector3Macro(Exclusion,int);
  
  vtkGetMacro(TotalVolume,int);
  vtkGetMacro(InclusionVolume,int);
  vtkGetMacro(ExclusionVolume,int);
  
protected:
//   virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
  
protected:
  vtkRectangularBoundingRegionFilter();
  virtual ~vtkRectangularBoundingRegionFilter();
  
private:
  int left()  {return Extent[0]+Inclusion[0];}
  int top()   {return Extent[2]+Inclusion[1];}
  int upper() {return Extent[4]+Inclusion[2];}
  int right() {return Extent[1]-Exclusion[0];}
  int bottom(){return Extent[3]-Exclusion[1];}
  int lower() {return Extent[5]-Exclusion[2];}
  
  int leftInclusion()  {return left()*Spacing[0];}
  int topInclusion()   {return top()*Spacing[1];}
  int upperInclusion() {return upper()*Spacing[2];}
  int rightInclusion() {return right()*Spacing[0];}
  int bottomInclusion(){return bottom()*Spacing[1];}
  int lowerInclusion() {return lower()*Spacing[2];}

private:
 // virtual vtkBoundingRegionFilter& operator=(const vtkBoundingRegionFilter& other); // Not implemented
 // virtual bool operator==(const vtkBoundingRegionFilter& other) const;// Not implemented
 int Extent[6];
 double Spacing[3];
 int Inclusion[3];
 int Exclusion[3];
 int TotalVolume;
 int InclusionVolume;
 int ExclusionVolume;
};

#endif // VTKRECTANGULARBOUNDINGREGIONFILTER_H
