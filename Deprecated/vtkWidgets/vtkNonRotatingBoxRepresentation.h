#ifndef VTKNONROTATINGBOXREPRESENTATION_H
#define VTKNONROTATINGBOXREPRESENTATION_H

#include "EspinaGUI_Export.h"

#include <vtkBoxRepresentation.h>

class EspinaGUI_EXPORT vtkNonRotatingBoxRepresentation
: public vtkBoxRepresentation
{
public:
  static vtkNonRotatingBoxRepresentation *New();  

  vtkTypeMacro(vtkNonRotatingBoxRepresentation, vtkBoxRepresentation); 

//   virtual void WidgetInteraction(double e[2]);
  virtual int ComputeInteractionState(int X, int Y, int modify = 0);
};

#endif
