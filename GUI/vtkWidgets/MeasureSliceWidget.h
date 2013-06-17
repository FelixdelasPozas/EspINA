/*
 * MeasureSliceWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef MEASURESLICEWIDGET_H_
#define MEASURESLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/vtkWidgets/EspinaWidget.h>

namespace EspINA
{
class EspinaGUI_EXPORT MeasureSliceWidget
: public SliceWidget
{
  public:
    explicit MeasureSliceWidget(vtkAbstractWidget *);
    virtual ~MeasureSliceWidget();

    virtual void SetEnabled(int);
};

}// namespace EspINA

#endif /* MEASURESLICEWIDGET_H_ */
