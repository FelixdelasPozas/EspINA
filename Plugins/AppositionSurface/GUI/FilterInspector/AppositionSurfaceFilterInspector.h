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

// ESPINA
#include <Filter/AppositionSurfaceFilter.h>
#include <Core/EspinaTypes.h>
#include <GUI/ViewManager.h>

class QUndoStack;

namespace ESPINA
{
  /// Filter Inspector
  //
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilterInspector
  : public Filter::FilterInspector
  {
    public:
      explicit AppositionSurfaceFilterInspector(FilterSPtr filter, EspinaModel *model);
      virtual ~AppositionSurfaceFilterInspector() {};

      virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager);

      class Widget;

    private:
      FilterSPtr m_filter;
      EspinaModel *m_model;
  };

  /// Filter Inspector Widget
  //
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceFilterInspector::Widget
  : public QWidget
  , public Ui::AppositionSurfaceFilterInspector
  {
      Q_OBJECT
    public:
      Widget(FilterSPtr filter, EspinaModel *model);
      ~Widget() {};

    private:
      AppositionSurfaceFilter::Pointer m_filter;
      EspinaModel *m_model;
  };


} /* namespace ESPINA */
#endif /* APPOSITIONSURFACEFILTERINSPECTOR_H_ */
