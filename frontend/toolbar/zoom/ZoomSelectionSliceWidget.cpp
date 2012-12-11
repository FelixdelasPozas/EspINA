/*
 * ZoomSelectionSliceWidget.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/ZoomSelectionSliceWidget.h"
#include "common/gui/ViewManager.h"

// vtk
#include <vtkAbstractWidget.h>

//----------------------------------------------------------------------------
ZoomSelectionSliceWidget::ZoomSelectionSliceWidget(vtkAbstractWidget *widget)
: SliceWidget(widget)
{
}

//----------------------------------------------------------------------------
ZoomSelectionSliceWidget::~ZoomSelectionSliceWidget()
{
}

//----------------------------------------------------------------------------
void ZoomSelectionSliceWidget::SetEnabled(int value)
{
  m_widget->SetEnabled(value);
}
