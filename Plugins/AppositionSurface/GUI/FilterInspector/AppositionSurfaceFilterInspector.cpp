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
  AppositionSurfaceFilterInspector::AppositionSurfaceFilterInspector(FilterPtr filter)
  : m_filter(filter)
  {
  }
  
  //----------------------------------------------------------------------------
  QWidget *AppositionSurfaceFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
  {
    // TODO
    return new AppositionSurfaceFilterInspector::Widget(reinterpret_cast<AppositionSurfaceFilter *>(m_filter));
  }

  /// Filter Inspector Widget
  //
  //----------------------------------------------------------------------------
  AppositionSurfaceFilterInspector::Widget::Widget(AppositionSurfaceFilter *filter)
  : m_filter(filter)
  {
    setupUi(this);

    m_origin->setText(m_filter->getOriginSegmentation());
    m_area->setText(QString().number(m_filter->getArea()));
    m_perimeter->setText(QString().number(m_filter->getPerimeter()));
    m_tortuosity->setText(QString().number(m_filter->getTortuosity()));
  }

} /* namespace EspINA */
