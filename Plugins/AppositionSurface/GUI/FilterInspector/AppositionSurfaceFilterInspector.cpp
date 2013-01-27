/*
 * AppositionSurfaceFilterInspector.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceFilterInspector.h"

namespace EspINA
{
  /// Filter Inspector
  //
  //----------------------------------------------------------------------------
  AppositionSurfaceFilterInspector::AppositionSurfaceFilterInspector(FilterSPtr filter)
  : m_filter(filter)
  {
  }
  
  //----------------------------------------------------------------------------
  QWidget *AppositionSurfaceFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
  {
    return new AppositionSurfaceFilterInspector::Widget(m_filter);
  }

  /// Filter Inspector Widget
  //
  //----------------------------------------------------------------------------
  AppositionSurfaceFilterInspector::Widget::Widget(FilterSPtr filter)
  {
    setupUi(this);

    m_filter = AppositionSurfaceFilter::Pointer(filter.data());
    m_origin->setText(m_filter->getOriginSegmentation());
    m_area->setText(QString().number(m_filter->getArea()));
    m_perimeter->setText(QString().number(m_filter->getPerimeter()));
    m_tortuosity->setText(QString().number(m_filter->getTortuosity()));
  }

} /* namespace EspINA */
