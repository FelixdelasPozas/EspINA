/*
 * ZoomSelectionSliceWidget.h
 *
 *  Created on: Nov 15, 2012
 *      Author: F�lix de las Pozas �lvarez
 */

#ifndef ZOOMSELECTIONSLICEWIDGET_H_
#define ZOOMSELECTIONSLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/vtkWidgets/EspinaWidget.h"

class vtkAbstractWidget;
class ViewManager;

namespace EspINA
{

class EspinaGUI_EXPORT ZoomSelectionSliceWidget
: public SliceWidget
{
  public:
    explicit ZoomSelectionSliceWidget(vtkAbstractWidget *);
    virtual ~ZoomSelectionSliceWidget();

    virtual void SetEnabled(int);
};

}// namespace EspINA

#endif /* ZOOMSELECTIONSLICEWIDGET_H_ */
