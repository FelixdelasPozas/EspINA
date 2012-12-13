/*
 * MeasureSliceWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef MEASURESLICEWIDGET_H_
#define MEASURESLICEWIDGET_H_

// EspINA
#include <GUI/vtkWidgets/EspinaWidget.h>

class MeasureSliceWidget
: public SliceWidget
{
  public:
    explicit MeasureSliceWidget(vtkAbstractWidget *);
    virtual ~MeasureSliceWidget();

    virtual void SetEnabled(int);
};

#endif /* MEASURESLICEWIDGET_H_ */
