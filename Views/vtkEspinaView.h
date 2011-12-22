/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef VTKESPINAVIEW_H
#define VTKESPINAVIEW_H

#include <vtkRenderViewBase.h>

#include <vtkSmartPointer.h>

class vtkProp;
class vtkRenderer;
class vtkRenderWindow;

class VTK_VIEWS_EXPORT vtkEspinaView : public vtkRenderViewBase
{
public:
  static vtkEspinaView *New();
  vtkTypeMacro(vtkEspinaView, vtkRenderViewBase);
  void PrintSelf(std::ostream& os, vtkIndent indent);

  // Sets/Gets the renderer for the area overview
  virtual vtkRenderer *GetOverviewRenderer();
  virtual void SetOverviewRenderer(vtkRenderer *ren);

  // Adds actor to renderer and to overview renderer
  void AddActor(vtkProp *actor);

  virtual void ResetCamera();
  virtual void ResetCameraClippingRange();

  void setSlice(unsigned int val);

  // Get a handle to the render window.
//   virtual vtkRenderWindow* GetRenderWindow();
  // Set the render window for this view. Note that this requires special
  // handling in order to do correctly - see the notes in the detailed
  // description of vtkRenderViewBase.
//   virtual void SetRenderWindow(vtkRenderWindow *win);

  // Updates the representations, then calls Render() on the render window
  // associated with this view.
//   virtual void Render();

protected:
  vtkEspinaView();
  virtual ~vtkEspinaView();

  // Called by the view when the renderer is about to render.
//   virtual void PrepareForRendering();

  vtkSmartPointer<vtkRenderer> OverviewRenderer;
  
private:
  vtkEspinaView (const vtkEspinaView& );// Not implemented
  void operator=(const vtkEspinaView& );// Not implemented
};

#endif // VTKESPINAVIEW_H
