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


#ifndef VTKCOUNTINGREGION_H
#define VTKCOUNTINGREGION_H

#include <vtkImageAlgorithm.h>
#include <vtkDenseArray.h>

class VTK_IMAGING_EXPORT vtkCountingRegionFilter
: public vtkImageAlgorithm
{
public:
  static vtkCountingRegionFilter *New();
  vtkTypeMacro(vtkCountingRegionFilter, vtkImageAlgorithm);
  
  //! User defined margins over bounding region
  vtkSetMacro(BottonMargin,double);
  vtkGetMacro(BottonMargin,double);
  vtkSetMacro(RightMargin,double);
  vtkGetMacro(RightMargin,double);
  vtkSetMacro(TopMargin,double);
  vtkGetMacro(TopMargin,double);
  vtkSetMacro(LeftMargin,double);
  vtkGetMacro(LeftMargin,double);
  
  
protected:
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  //virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

protected:
  vtkCountingRegionFilter();
  virtual ~vtkCountingRegionFilter();
  
private:
  double BottonMargin;
  double RightMargin;
  double TopMargin;
  double LeftMargin;
};

#endif // VTKCOUNTINGREGION_H
