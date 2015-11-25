/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef VTK_COUNTING_FRAME_SLICE_WIDGET_H
#define VTK_COUNTING_FRAME_SLICE_WIDGET_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include "vtkCountingFrameWidget.h"

// VTK
#include <vtkWidgetRepresentation.h>

class vtkPolyData;
class vtkCountingFrameSliceRepresentation;

class CountingFramePlugin_EXPORT vtkCountingFrameSliceWidget
: public vtkCountingFrameWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkCountingFrameSliceWidget *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeMacro(vtkCountingFrameSliceWidget, vtkCountingFrameWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /** \brief Sets the plane of the widget.
   *
   */
  virtual void SetPlane(ESPINA::Plane plane);

  /** \brief Sets the depth of the widget in the plane direction.
   * \param[in] depth depth in Nm.
   *
   */
  virtual void SetRepresentationDepth(ESPINA::Nm depth);

  /** \brief Sets the slice the widget must represent.
   * \param[in] pos slice in Nm.
   *
   */
  virtual void SetSlice(ESPINA::Nm pos);

  virtual void SetCountingFrame(vtkSmartPointer<vtkPolyData> cf,
                                ESPINA::Nm   inclusionOffset[3],
                                ESPINA::Nm   exclusionOffset[3],
                                ESPINA::NmVector3 resolution);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkRectangularCountingFrameRepresentation class.
  void CreateDefaultRepresentation();

  /** \brief Highlights the representation.
   * \param[in] highlight true to highlight false otherwise.
   *
   */
  void SetHighlighted(bool highlight);

  virtual void setVisible(bool visible);

protected:
  vtkCountingFrameSliceWidget();

  ~vtkCountingFrameSliceWidget();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0, Active};
//ETX

  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);

  static void EndSelectAction(vtkAbstractWidget*);

  static void TranslateAction(vtkAbstractWidget*);

  static void MoveAction(vtkAbstractWidget*);

  // helper methods for cursoe management
  virtual void SetCursor(int state);

  ESPINA::Plane     Plane;
  ESPINA::Nm        Slice;
  ESPINA::NmVector3 SlicingStep;
  ESPINA::Nm        Depth;

private:
  vtkCountingFrameSliceWidget(const vtkCountingFrameSliceWidget&);  //Not implemented
  void operator=(const vtkCountingFrameSliceWidget&);  //Not implemented

  static void centerMarginsOnVoxelCenter(vtkCountingFrameSliceWidget* self);
};

#endif //VTKRBOUNDINGFRAMESLICEWIDGET_H
