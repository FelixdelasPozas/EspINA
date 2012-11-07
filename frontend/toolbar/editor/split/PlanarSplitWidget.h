/*
 * PlanarSplitWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef PLANARSPLITWIDGET_H_
#define PLANARSPLITWIDGET_H_

class PlanarSplitSliceWidget;

// EspINA
#include "common/widgets/EspinaWidget.h"

// vtk
#include <vtkSmartPointer.h>

// Qt
#include <QObject>
#include <QList>

class vtkAbstractWidget;
class vtkPoints;

class PlanarSplitWidget
: public EspinaWidget
{
  public:
    explicit PlanarSplitWidget();
    virtual ~PlanarSplitWidget();

    // EspinaWidget implementation
    virtual vtkAbstractWidget *createWidget();
    virtual void deleteWidget(vtkAbstractWidget *widget);
    virtual SliceWidget *createSliceWidget(PlaneType plane);

    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    // get/set
    virtual void setPlanePoints(vtkSmartPointer<vtkPoints>);
    virtual vtkSmartPointer<vtkPoints> getPlanePoints();

  private:
    PlanarSplitSliceWidget *m_axialWidget;
    PlanarSplitSliceWidget *m_coronalWidget;
    PlanarSplitSliceWidget *m_sagittalWidget;
    QList<vtkAbstractWidget*> m_widgets;
    PlaneType m_mainWidget;
};

#endif /* PLANARSPLITWIDGET_H_ */
