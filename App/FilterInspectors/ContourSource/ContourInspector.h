/*
 * ContourInspector.h
 *
 *  Created on: Sep 30, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CONTOURINSPECTOR_H_
#define CONTOURINSPECTOR_H_

#include <Core/Model/Filter.h>

class QWidget;
class QUndoStack;

namespace EspINA
{
  class ViewManager;
  class ContourSource;

  class ContourFilterInspector
  : public Filter::FilterInspector
  {
    public:
      ContourFilterInspector(FilterPtr filter);

      virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager);

      class Widget;

    private:
      FilterPtr  m_filter;
  };

} // namespace EspINA

#endif /* CONTOURINSPECTOR_H_ */
