/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#ifndef VTKINTERACTORSTYLEESPINASLICE_H
#define VTKINTERACTORSTYLEESPINASLICE_H

#include "EspinaGUI_Export.h"

#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

// Interactor Style to be used with Slice Views
class EspinaGUI_EXPORT vtkInteractorStyleEspinaSlice
: public vtkInteractorStyleImage
{
public:
  static vtkInteractorStyleEspinaSlice *New();
  vtkTypeMacro(vtkInteractorStyleEspinaSlice, vtkInteractorStyleImage);

  // Disable mouse wheel
  virtual void OnMouseWheelForward() {}
  virtual void OnMouseWheelBackward() {}

  // Disable keyboard
  virtual void OnKeyPress(){}
  virtual void OnKeyRelease(){}
  virtual void OnKeyUp(){}
  virtual void OnKeyDown(){}

  // Disable modifying brightness and saturation
  virtual void OnLeftButtonDown() {}
  virtual void OnLeftButtonUp() {}
  //   virtual void OnMouseMove();
  // Disable zoom if Ctrl is pressed
  virtual void OnRightButtonDown();
protected:
  explicit vtkInteractorStyleEspinaSlice()
  {
    AutoAdjustCameraClippingRangeOn();
    KeyPressActivationOff();
  }
  virtual ~vtkInteractorStyleEspinaSlice(){}

private:
  vtkInteractorStyleEspinaSlice(const vtkInteractorStyleEspinaSlice&); // Not implemented
  void operator=(const vtkInteractorStyleEspinaSlice&);// Not implemented
};

using View2DInteractor = vtkSmartPointer<vtkInteractorStyleEspinaSlice>;

#endif // VTKINTERACTORSTYLEESPINASLICE_H
