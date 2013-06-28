/*
 * MeasureSliceWidget.cpp
 *
 *  Created on: Dec 12, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "MeasureSliceWidget.h"

// VTK
#include <vtkAbstractWidget.h>

using namespace EspINA;

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
