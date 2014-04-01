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
#include <vtkCommand.h>

class QEvent;
class vtkAbstractWidget;

namespace EspINA
{
  class RenderView;

  class EspinaGUI_EXPORT ZoomSelectionWidget
  : public EspinaWidget
  , public EventHandler
  , public vtkCommand
  {
  public:
    /* \brief ZoomSelectionWidget class destructor.
     *
     */
    virtual ~ZoomSelectionWidget();

    vtkTypeMacro(ZoomSelectionWidget, vtkCommand);

    /* \brief VTK-style class New() method.
     *
     */
    static ZoomSelectionWidget *New()
    { return new ZoomSelectionWidget(); }

    /* \brief Implements EspinaWidget::registerView()
     *
     */
    virtual void registerView(RenderView *);

    /* \brief Implements EspinaWidget::unregisterView()
     *
     */
    virtual void unregisterView(RenderView *);

    /* \brief Process events from the vtkRenderWindowInteractor
     *
     */
    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);

    /* \brief Implements EspinaWidget::setEnabled
     *
     */
    virtual void setEnabled(bool enable);

    /* \brief Implements vtkCommand::Execute.
     *
     */
    void Execute(vtkObject *, unsigned long int, void*);

    /* \brief Implements EventHandler::filterEvent.
     *
     */
    virtual bool filterEvent(QEvent *e, RenderView *view);

  private:
    /* \brief ZoomSelectionWidget class destructor, private.
     *
     */
    explicit ZoomSelectionWidget();

  private:
    QMap<RenderView *, vtkZoomSelectionWidget *> m_views;
  };

}// namespace EspINA

#endif /* ZOOMSELECTIONWIDGET_H_ */
