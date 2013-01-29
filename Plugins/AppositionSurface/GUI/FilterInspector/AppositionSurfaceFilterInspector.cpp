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

    double area = m_filter->getArea();
    double perimeter = m_filter->getPerimeter();
    double tortuosity = m_filter->getTortuosity();

    if (-1 != area)
      m_area->setText(QString().number(m_filter->getArea()));
    else
      m_area->setText("Not computed");

    if (-1 != perimeter)
      m_perimeter->setText(QString().number(m_filter->getPerimeter()));
    else
      m_perimeter->setText("Not computed");

    if (-1 != tortuosity)
      m_tortuosity->setText(QString().number(m_filter->getTortuosity()));
    else
      m_tortuosity->setText("Not computed");
  }

} /* namespace EspINA */
