/*
 * PlanarSplitSliceWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef PLANARSPLITSLICEWIDGET_H_
#define PLANARSPLITSLICEWIDGET_H_

// EspINA
#include "common/widgets/EspinaWidget.h"
#include "common/EspinaTypes.h"

// vtk
#include <vtkSmartPointer.h>

// forward declarations
class vtkPoints;
class vtkPlanarSplitWidget;

class PlanarSplitSliceWidget
: public SliceWidget
{
  public:
    explicit PlanarSplitSliceWidget(vtkAbstractWidget *widget);
    virtual ~PlanarSplitSliceWidget();

    virtual void setSlice(Nm pos, PlaneType plane);

    virtual void setOrientation(PlaneType);

    virtual void setPoints(vtkSmartPointer<vtkPoints>);
    vtkSmartPointer<vtkPoints> getPoints();

    virtual void setEnabled(bool);

    virtual vtkAbstractWidget* getWidget() { return this->m_widget; };
    virtual void disableWidget();
  private:
    PlaneType m_plane;
    bool m_mainWidget;
};

#endif /* PLANARSPLITSLICEWIDGET_H_ */
