/*
 * MeasureWidget.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_MEASURE_WIDGET_H_
#define ESPINA_MEASURE_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/EventHandler.h>

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

class vtkAbstractWidget;
class vtkDistanceWidget;
class vtkMeasureWidget;
class vtkCamera;
class vtkRenderWindowInteractor;

class QEvent;

namespace ESPINA
{
  class RenderView;
  class MeasureWidget;
  class vtkDistanceCommand;

  class EspinaGUI_EXPORT MeasureWidget
  : public EspinaWidget
  , public EventHandler
  {
  public:
    /* \brief Class MeasureWidget destructor.
     *
     */
    explicit MeasureWidget();

    /* \brief MeasureWidget class destructor.
     *
     */
    virtual ~MeasureWidget();

    /* \brief Implements EspinaWidget::registerView()
     *
     */
    virtual void registerView(RenderView *view);

    /* \brief Implements EspinaWidget::unregisterView()
     *
     */
    virtual void unregisterView(RenderView *view);

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

  private:
    friend class vtkDistanceCommand;

    /* \brief Computes optimal tick distance in the ruler given the length.
     *
     */
    double ComputeRulerTickDistance(double);

    vtkSmartPointer<vtkDistanceCommand>          m_command;
    QMap<vtkDistanceWidget *, QList<vtkCamera*>> m_cameras;
    QMap<RenderView *, vtkDistanceWidget *>      m_widgets;

  };

  class vtkDistanceCommand
  : public vtkCommand
  {
    vtkTypeMacro(vtkDistanceCommand, vtkCommand);

    /* \brief VTK-style New() constructor, required for using vtkSmartPointer.
     *
     */
    static vtkDistanceCommand* New()
    { return new vtkDistanceCommand(); }

    /* \brief Implements vtkEspinaCommand::Execute
     *
     */
    virtual void Execute(vtkObject *, unsigned long int, void*);

    /* \brief Implements vtkEspinaCommand::setWidget
     *
     */
    void setWidget(EspinaWidgetPtr widget)
    { m_widget = dynamic_cast<MeasureWidget *>(widget); }

    private:
     /* \brief Class vtkDistanceCommand class private constructor.
      *
      */
     explicit vtkDistanceCommand()
     : m_widget{nullptr}
     {}

     /* \brief Class vtkDistanceCommand class private destructor.
      *
      */
     virtual ~vtkDistanceCommand()
     {}

     ESPINA::MeasureWidget *m_widget;
  };

}// namespace ESPINA

#endif // ESPINA_MEASURE_WIDGET_H_
