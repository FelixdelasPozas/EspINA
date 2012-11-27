/*
 * ZoomSelectionWidget.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ZOOMSELECTIONWIDGET_H_
#define ZOOMSELECTIONWIDGET_H_

// EspINA
#include "GUI/vtkWidgets/vtkZoomSelectionWidget.h"
#include "GUI/vtkWidgets/EspinaWidget.h"

// Qt
#include <QList>

// vtk
#include <vtkCommand.h>

class vtkAbstractWidget;
class ZoomSelectionSliceWidget;

class ZoomSelectionWidget
: public EspinaWidget
, public vtkCommand
{
  public:
    explicit ZoomSelectionWidget();
    virtual ~ZoomSelectionWidget();

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

  private:
    ZoomSelectionSliceWidget *m_axial;
    ZoomSelectionSliceWidget *m_coronal;
    ZoomSelectionSliceWidget *m_sagittal;
    vtkZoomSelectionWidget   *m_volume;
    QList<vtkAbstractWidget*> m_widgets;
};

#endif /* ZOOMSELECTIONWIDGET_H_ */
