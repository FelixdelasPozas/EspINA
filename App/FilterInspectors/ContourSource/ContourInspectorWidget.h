/*
 * ContourInspectorWidget.h
 *
 *  Created on: Jan 14, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CONTOURINSPECTORWIDGET_H_
#define CONTOURINSPECTORWIDGET_H_

#include <QWidget>
#include <App/FilterInspectors/ContourSource/ContourInspector.h>
#include "ui_ContourInspectorWidget.h"

namespace EspINA
{
  class ContourSource;
  
  class ContourFilterInspector::Widget
  : public QWidget
  , public Ui::ContourInspectorWidget
  {
    Q_OBJECT
    public:
      explicit Widget(ContourSource *source);
      virtual ~Widget();

    protected slots:
      void EditContours();
      void UpdateValues();

    private:
      ContourSource *m_source;
  };

} /* namespace EspINA */

#endif /* CONTOURINSPECTORWIDGET_H_ */
