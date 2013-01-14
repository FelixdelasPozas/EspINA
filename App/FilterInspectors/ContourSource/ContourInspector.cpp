/*
 * ContourSourceInspector.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#include "ContourInspector.h"
#include "ContourInspectorWidget.h"

using namespace EspINA;

//----------------------------------------------------------------------------
ContourFilterInspector::ContourFilterInspector(FilterPtr filter)
: m_filter(filter)
{
}

//----------------------------------------------------------------------------
QWidget *ContourFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
{
  return new ContourFilterInspector::Widget(reinterpret_cast<ContourSource*>(m_filter));
}
