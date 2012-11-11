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
#include "common/EspinaTypes.h"

// vtk
#include <vtkSmartPointer.h>
#include <vtkCommand.h>

// Qt
#include <QList>

class vtkAbstractWidget;
class vtkPoints;
class vtkPlane;
class vtkAbstractWidget;
class vtkImageStencilSource;

enum WidgetType { AXIAL_WIDGET = 1, CORONAL_WIDGET, SAGITTAL_WIDGET, VOLUME_WIDGET, NONE };

class PlanarSplitWidget
: public EspinaWidget
, public vtkCommand
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
    virtual vtkSmartPointer<vtkPlane> getImplicitPlane();
    virtual void setSegmentationBounds(double *);
    virtual bool planeIsValid();

    virtual WidgetType getMainWidget();

    // vtkCommand
    virtual void Execute (vtkObject *caller, unsigned long eventId, void *callData);

  private:
    PlanarSplitSliceWidget *m_axial;
    PlanarSplitSliceWidget *m_coronal;
    PlanarSplitSliceWidget *m_sagittal;
    QList<vtkAbstractWidget*> m_widgets;
    WidgetType m_mainWidget;
};

#endif /* PLANARSPLITWIDGET_H_ */
