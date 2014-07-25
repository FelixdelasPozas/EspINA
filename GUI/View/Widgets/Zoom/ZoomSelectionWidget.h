/*
 * ZoomSelectionWidget.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_ZOOM_SELECTION_WIDGET_H_
#define ESPINA_ZOOM_SELECTION_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/View/EventHandler.h>

// Qt
#include <QList>

// vtk
#include <vtkSmartPointer.h>

class QEvent;
class vtkAbstractWidget;

namespace ESPINA
{
  class RenderView;
  class vtkZoomCommand;

  using ZoomSelectionWidgetAdapter = EspinaInteractorAdapter<vtkZoomSelectionWidget>;

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

    /* \brief Implements EventHandler::setInUse
     *
     */
    void setInUse(bool value);

  private:
    friend class vtkZoomCommand;

    /* \brief ZoomSelectionWidget class constructor.
     *
     */
    explicit ZoomSelectionWidget();

    vtkSmartPointer<vtkZoomCommand>              m_command;
    QMap<RenderView *, ZoomSelectionWidgetAdapter *> m_views;

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

}// namespace ESPINA

#endif // ESPINA_ZOOM_SELECTION_WIDGET_H_
