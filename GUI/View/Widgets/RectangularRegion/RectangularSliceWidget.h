/*
 * RectangularSliceWidget.h
 *
 *  Created on: 21/05/2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef RECTANGULARSLICEWIDGET_H_
#define RECTANGULARSLICEWIDGET_H_

#include "EspinaGUI_Export.h"

#include "vtkRectangularSliceWidget.h"
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/EspinaTypes.h>

namespace EspINA
{
	class EspinaGUI_EXPORT RectangularSliceWidget
	: public SliceWidget
	{
	public:
	    explicit RectangularSliceWidget(vtkRectangularSliceWidget *widget);
	    ~RectangularSliceWidget();
	    virtual void setSlice(Nm pos, PlaneType plane);
	};
};


#endif /* RECTANGULARSLICEWIDGET_H_ */
