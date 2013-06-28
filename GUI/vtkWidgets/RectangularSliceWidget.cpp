/*
 * RectangularSliceWidget.cpp
 *
 *  Created on: 21/05/2013
 *      Author: F�lix de las Pozas �lvarez
 */

#include "GUI/vtkWidgets/RectangularSliceWidget.h"

namespace EspINA
{
  //----------------------------------------------------------------------------
  RectangularSliceWidget::RectangularSliceWidget(vtkRectangularSliceWidget *widget)
  : SliceWidget(widget)
  {
  }

  //----------------------------------------------------------------------------
  RectangularSliceWidget::~RectangularSliceWidget()
  {
  }

  //----------------------------------------------------------------------------
  void RectangularSliceWidget::setSlice(Nm pos, PlaneType plane)
  {
     reinterpret_cast<vtkRectangularSliceWidget*>(m_widget)->SetSlice(pos);
  }
}
