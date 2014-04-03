/*
 * ZoomSelectionWidget.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ZOOMSELECTIONWIDGET_H_
#define ZOOMSELECTIONWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/ViewManager.h>

// Qt
#include <QList>

// vtk
#include <vtkSmartPointer.h>

class QEvent;
class vtkAbstractWidget;

namespace EspINA
{
  class RenderView;
  class vtkZoomCommand;

  class EspinaGUI_EXPORT ZoomSelectionWidget
  : public EspinaWidget
  , public EventHandler
  {
  public:
    /* \brief VTK-style class New() method
     *
     */
    static ZoomSelectionWidget *New()
    { return new ZoomSelectionWidget(); }

    /* \brief ZoomSelectionWidget class destructor.
     *
     */
    virtual ~ZoomSelectionWidget();

    /* \brief Implements EspinaWidget::registerView()
     *
     */
    virtual void registerView(RenderView *);

    /* \brief Implements EspinaWidget::unregisterView()
     *
     */
    virtual void unregisterView(RenderView *);

    /* \brief Implements EspinaWidget::setEnabled
     *
     */
    virtual void setEnabled(bool enable);

    /* \brief Implements EventHandler::filterEvent
     *
     */
    bool filterEvent(QEvent *e, RenderView *view);

  private:
    friend class vtkZoomCommand;

    /* \brief ZoomSelectionWidget class constructor.
     *
     */
    explicit ZoomSelectionWidget();

    vtkSmartPointer<vtkZoomCommand>              m_command;
    QMap<RenderView *, vtkZoomSelectionWidget *> m_views;
  };

  class vtkZoomCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkZoomCommand, vtkEspinaCommand);

      /* \brief VTK-style New() constructor, required for using vtkSmartPointer.
       *
       */
      static vtkZoomCommand *New()
      { return new vtkZoomCommand(); }

      /* \brief Implements vtkEspinaCommand::Execute.
       *
       */
      void Execute(vtkObject *, unsigned long int, void*);

      /* \brief Implements vtkEspinaCommand::setWidget();
       *
       */
      void setWidget(EspinaWidgetPtr widget)
      { m_widget = dynamic_cast<ZoomSelectionWidget *>(widget); }

    private:
      /* \brief vtkZoomCommand class private constructor.
       *
       */
      explicit vtkZoomCommand()
      : m_widget{nullptr}
      {}

      /* \brief vtkZoomCommand class private destructor.
       *
       */
      virtual ~vtkZoomCommand()
      {};

      ZoomSelectionWidget *m_widget;
  };

}// namespace EspINA

#endif /* ZOOMSELECTIONWIDGET_H_ */
