/*
 * AppositionSurfaceFilterInspector.h
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACEFILTERINSPECTOR_H_
#define APPOSITIONSURFACEFILTERINSPECTOR_H_

#include "AppositionSurfacePlugin_Export.h"

#include "ui_AppositionSurfaceFilterInspector.h"

// EspINA
#include <Filter/AppositionSurfaceFilter.h>
#include <Core/EspinaTypes.h>
#include <GUI/ViewManager.h>

class QUndoStack;

namespace EspINA
{
  /// Filter Inspector
  //
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilterInspector
  : public Filter::FilterInspector
  {
    public:
      explicit AppositionSurfaceFilterInspector(FilterSPtr filter);
      virtual ~AppositionSurfaceFilterInspector() {};

      virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager);

      class Widget;

    private:
      FilterSPtr m_filter;
  };

  /// Filter Inspector Widget
  //
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilterInspector::Widget
  : public QWidget
  , public Ui::AppositionSurfaceFilterInspector
  {
      Q_OBJECT
    public:
      Widget(FilterSPtr filter);
      ~Widget() {};

    private:
      AppositionSurfaceFilter::Pointer m_filter;
  };


} /* namespace EspINA */
#endif /* APPOSITIONSURFACEFILTERINSPECTOR_H_ */
