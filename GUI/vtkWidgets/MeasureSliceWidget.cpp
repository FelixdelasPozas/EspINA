/*
 * MeasureSliceWidget.cpp
 *
 *  Created on: Dec 12, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "MeasureSliceWidget.h"

#include <GUI/ViewManager.h>

// vtk
#include <vtkAbstractWidget.h>

//----------------------------------------------------------------------------
MeasureSliceWidget::MeasureSliceWidget(vtkAbstractWidget *widget)
: SliceWidget(widget)
{
}

//----------------------------------------------------------------------------
MeasureSliceWidget::~MeasureSliceWidget()
{
}

//----------------------------------------------------------------------------
void MeasureSliceWidget::SetEnabled(int value)
{
  m_widget->SetEnabled(value);
}
