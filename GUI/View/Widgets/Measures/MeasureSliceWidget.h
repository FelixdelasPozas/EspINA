/*
 * MeasureSliceWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef MEASURESLICEWIDGET_H_
#define MEASURESLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/View/Widgets/EspinaWidget.h>

namespace EspINA
{
class EspinaGUI_EXPORT MeasureSliceWidget
: public SliceWidget
{
  public:
    /* \brief MeasureSliceWidget class constructor.
     * \param[in] widget vtkAbstractWidget to encapsulate.
     *
     */
    explicit MeasureSliceWidget(vtkAbstractWidget *widget);

    /* \brief MeasureSliceWidget class destructor.
     *
     */
    virtual ~MeasureSliceWidget();

    /* \brief Enables/disables this widget and the one it encapsulates.
     * \param[in] value int value to pass to the vtkAbstractWidget.
     *
     */
    virtual void SetEnabled(int value);
};

}// namespace EspINA

#endif /* MEASURESLICEWIDGET_H_ */
