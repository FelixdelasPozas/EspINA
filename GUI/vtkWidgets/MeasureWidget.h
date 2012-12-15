/*
 * MeasureWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef MEASUREWIDGET_H_
#define MEASUREWIDGET_H_

// EspINA
#include <GUI/vtkWidgets/EspinaWidget.h>

// Qt
#include <QList>

// vtk
#include <vtkCommand.h>

class vtkAbstractWidget;
class vtkDistanceWidget;
class MeasureSliceWidget;
class vtkMeasureWidget;
class EspinaRenderView;
class QEvent;

class MeasureWidget
: public EspinaWidget
, public vtkCommand
{
  public:
    explicit MeasureWidget();
    virtual ~MeasureWidget();

    // implements EspinaWidget
    void setViewManager(ViewManager *vm) {m_viewManager = vm;}
    virtual vtkAbstractWidget *createWidget();
    virtual void deleteWidget(vtkAbstractWidget *widget);
    virtual SliceWidget *createSliceWidget(PlaneType plane);

    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    // implements vtkCommand
    void Execute(vtkObject *, unsigned long int, void*);

    bool filterEvent(QEvent *e, EspinaRenderView *view);

  private:
    // helper methods
    double ComputeRulerTickDistance(double);

    vtkDistanceWidget *m_axial;
    vtkDistanceWidget *m_coronal;
    vtkDistanceWidget *m_sagittal;
    QList<MeasureSliceWidget*> m_sliceWidgets;
    QList<vtkDistanceWidget*> m_distanceWidgets;
};

#endif /* MEASUREWIDGET_H_ */
