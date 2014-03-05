/*
 * ZoomSelectionSliceWidget.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ZOOMSELECTIONSLICEWIDGET_H_
#define ZOOMSELECTIONSLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/View/Widgets/EspinaWidget.h>

class vtkAbstractWidget;
class ViewManager;

namespace EspINA
{

class EspinaGUI_EXPORT ZoomSelectionSliceWidget
: public SliceWidget
{
  public:
    /* \brief ZoomSelectionSliceWidget class constructor.
     *
     */
    explicit ZoomSelectionSliceWidget(vtkAbstractWidget *);

    /* \brief ZoomSelectionSliceWidget class destructor.
     *
     */
    virtual ~ZoomSelectionSliceWidget();

    /* \brief Enables/Disables the vtkAbstractWidget contained.
     * \param[in] value int value to pass to the vtkAbstractWidget.
     */
    virtual void SetEnabled(int value);
};

}// namespace EspINA

#endif /* ZOOMSELECTIONSLICEWIDGET_H_ */
