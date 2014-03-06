/*
 * MeasureWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_MEASURE_WIDGET_H_
#define ESPINA_MEASURE_WIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/ViewManager.h>

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
  class RenderView;

  class EspinaGUI_EXPORT MeasureWidget
  : public EspinaWidget
  , public vtkCommand
  {
  public:
    /* \brief MeasureWidget class destructor.
     *
     */
    virtual ~MeasureWidget();

    vtkTypeMacro(MeasureWidget,vtkCommand);

    /* \brief VTK-style class New() method
     *
     */
    static MeasureWidget *New()
    {return new MeasureWidget();};

    /* \brief Sets the widget view manager.
     * \param[in] vm Application view manager.
     */
    void setViewManager(ViewManagerSPtr vm)
    { m_viewManager = vm; }

    /* \brief Implements EspinaWidget::create3DWidget.
     *
     */
    virtual vtkAbstractWidget *create3DWidget(View3D *view);

    /* \brief Implements EspinaWidget::createSliceWidget.
     *
     */
    virtual SliceWidget *createSliceWidget(View2D *view);

    /* \brief Implements EspinaWidget::processEvents.
     *
     */
    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);

    /* \brief Implements EspinaWidget::setEnabled.
     *
     */
    virtual void setEnabled(bool enable);

    /* \brief Implements EspinaWidget::filterEvent.
     *
     */
    bool filterEvent(QEvent *e, RenderView *view);

    /* \brief Implements vtkCommand::Execute.
     *
     */
    void Execute(vtkObject *, unsigned long int, void*);

  private:
    explicit MeasureWidget();

    /* \brief Computes optimal tick distance in the ruler given the length.
     *
     */
    double ComputeRulerTickDistance(double);

    vtkDistanceWidget *m_axial;
    vtkDistanceWidget *m_coronal;
    vtkDistanceWidget *m_sagittal;
    QList<MeasureSliceWidget*> m_sliceWidgets;
    QList<vtkDistanceWidget*> m_distanceWidgets;
    ViewManagerSPtr    m_viewManager;

    QList<vtkCamera*> m_axialCameras;
    QList<vtkCamera*> m_coronalCameras;
    QList<vtkCamera*> m_sagittalCameras;
  };

}// namespace EspINA

#endif // ESPINA_MEASURE_WIDGET_H_
