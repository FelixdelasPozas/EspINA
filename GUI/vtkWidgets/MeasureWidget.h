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
class vtkMeasureWidget;
class vtkCamera;
class QEvent;

namespace EspINA
{
  class MeasureSliceWidget;
  class EspinaRenderView;

  class MeasureWidget
  : public EspinaWidget
  , public vtkCommand
  {
  public:
    virtual ~MeasureWidget();

    vtkTypeMacro(MeasureWidget,vtkCommand);

    static MeasureWidget *New()
    {return new MeasureWidget;};

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
    explicit MeasureWidget();

    // helper methods
    double ComputeRulerTickDistance(double);

    vtkDistanceWidget *m_axial;
    vtkDistanceWidget *m_coronal;
    vtkDistanceWidget *m_sagittal;
    QList<MeasureSliceWidget*> m_sliceWidgets;
    QList<vtkDistanceWidget*> m_distanceWidgets;
    
    QList<vtkCamera*> m_axialCameras;
    QList<vtkCamera*> m_coronalCameras;
    QList<vtkCamera*> m_sagittalCameras;
  };

}// namespace EspINA

#endif /* MEASUREWIDGET_H_ */
