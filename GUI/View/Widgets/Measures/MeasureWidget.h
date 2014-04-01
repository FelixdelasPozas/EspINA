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
#include <Support/EventHandler.h>

// Qt
#include <QMap>

// vtk
#include <vtkCommand.h>

class vtkAbstractWidget;
class vtkDistanceWidget;
class vtkMeasureWidget;
class vtkCamera;
class vtkRenderWindowInteractor;

class QEvent;

namespace EspINA
{
  class RenderView;

  class EspinaGUI_EXPORT MeasureWidget
  : public EspinaWidget
  , public EventHandler
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
    { return new MeasureWidget(); };

    /* \brief Implements EspinaWidget::registerView()
     *
     */
    virtual void registerView(RenderView *view);

    /* \brief Implements EspinaWidget::unregisterView()
     *
     */
    virtual void unregisterView(RenderView *view);

    /* \brief Process events from vtkRenderWindowInteractor.
     *
     */
    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);

    /* \brief Implements EspinaWidget::setEnabled.
     *
     */
    virtual void setEnabled(bool enable);

    /* \brief Implements EventHandler::filterEvent.
     *
     */
    bool filterEvent(QEvent *e, RenderView *view);

    /* \brief Implements EventHandler::setInUse()
     *
     */
    void setInUse(bool value);

    /* \brief Implements vtkCommand::Execute.
     *
     */
    void Execute(vtkObject *, unsigned long int, void*);

  private:
    /* \brief Class MeasureWidget destructor.
     *
     */
    explicit MeasureWidget();

    /* \brief Computes optimal tick distance in the ruler given the length.
     *
     */
    double ComputeRulerTickDistance(double);

    QMap<vtkDistanceWidget *, QList<vtkCamera*>> m_cameras;
    QMap<RenderView *, vtkDistanceWidget *> m_widgets;
  };

}// namespace EspINA

#endif // ESPINA_MEASURE_WIDGET_H_
