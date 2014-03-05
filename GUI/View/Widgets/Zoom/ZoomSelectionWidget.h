/*
 * ZoomSelectionWidget.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
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

class vtkAbstractWidget;

namespace EspINA
{
  class ZoomSelectionSliceWidget;

  class EspinaGUI_EXPORT ZoomSelectionWidget
  : public EspinaWidget
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

    /* \brief Sets the view manager for the widget, only needed to refresh the views.
     * \param[in] vm Application view manager.
     */
    void setViewManager(ViewManagerSPtr vm)
    { m_viewManager = vm;}

    /* \brief Implements EspinaWidget::create3DWidget, currently zoom is disabled for 3D view.
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

    /* \brief Implements EspinaWidget::setEnabled
     *
     */
    virtual void setEnabled(bool enable);

    /* \brief Implements vtkCommand::Execute.
     *
     */
    void Execute(vtkObject *, unsigned long int, void*);

  private:
    /* \brief ZoomSelectionWidget class destructor, private.
     *
     */
    explicit ZoomSelectionWidget();

  private:
    ZoomSelectionSliceWidget *m_axial;
    ZoomSelectionSliceWidget *m_coronal;
    ZoomSelectionSliceWidget *m_sagittal;
    vtkZoomSelectionWidget   *m_volume;
    QList<vtkAbstractWidget*> m_widgets;
    ViewManagerSPtr           m_viewManager;
  };

}// namespace EspINA

#endif /* ZOOMSELECTIONWIDGET_H_ */
