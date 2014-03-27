/*
 * AppositionSurfaceFilterInspector.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: Felix de las Pozas Alvarez
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

    m_filter = dynamic_cast<AppositionSurfaceFilter *>(filter.get());
    m_origin->setText(m_filter->getOriginSegmentation());

    double area =  0; // m_filter->getArea();
    double perimeter = 0; // m_filter->getPerimeter();
    double tortuosity = 0; //m_filter->getTortuosity();

    if (-1 != area)
      m_area->setText(QString::number(area));
    else
      m_area->setText("Not computed");

    if (-1 != perimeter)
      m_perimeter->setText(QString::number(perimeter));
    else
      m_perimeter->setText("Not computed");

    if (-1 != tortuosity)
      m_tortuosity->setText(QString::number(tortuosity));
    else
      m_tortuosity->setText("Not computed");
  }

} /* namespace EspINA */
