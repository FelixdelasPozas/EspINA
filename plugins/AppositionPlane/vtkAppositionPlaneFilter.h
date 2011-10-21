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


#ifndef VTKAPPOSITIONPLANEFILTER_H
#define VTKAPPOSITIONPLANEFILTER_H

#include <vtkPolyDataAlgorithm.h>


/// Implementation for Apposition Plane Filter 
/// designed by Juan Morales del Olmo
class VTK_IMAGING_EXPORT vtkAppositionPlaneFilter
      : public vtkPolyDataAlgorithm
{
public:
  static vtkAppositionPlaneFilter *New();
  vtkTypeMacro(vtkAppositionPlaneFilter, vtkPolyDataAlgorithm);

  /// Apposition Plane Resolution
  vtkSetMacro(Resolution,int);
  vtkGetMacro(Resolution,int);
  
protected:
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

protected:
  vtkAppositionPlaneFilter();
  virtual ~vtkAppositionPlaneFilter();

private:
 // virtual vtkBoundingRegionFilter& operator=(const vtkBoundingRegionFilter& other); // Not implemented
 // virtual bool operator==(const vtkBoundingRegionFilter& other) const;// Not implemented
 int Resolution;
 int NumIterations;//TODO: change to unsigned if possible
 bool Converge;
};

#endif // VTKAPPOSITIONPLANEFILTER_H
