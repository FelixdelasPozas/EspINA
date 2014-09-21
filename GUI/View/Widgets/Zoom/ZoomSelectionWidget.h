/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    /** \brief VTK-style class New() method
     *
     */
    static ZoomSelectionWidget *New()
    { return new ZoomSelectionWidget(); }

    /** \brief ZoomSelectionWidget class destructor.
     *
     */
    virtual ~ZoomSelectionWidget();

    /** \brief Implements EspinaWidget::registerView().
     *
     */
    virtual void registerView(RenderView *);

    /** \brief Implements EspinaWidget::unregisterView().
     *
     */
    virtual void unregisterView(RenderView *);

    /** \brief Implements EspinaWidget::setEnabled()
     *
     */
    virtual void setEnabled(bool enable);

    /** \brief Overrides EventHandler::filterEvent
     *
     */
    bool filterEvent(QEvent *e, RenderView *view) override;

    /** \brief Overrides EventHandler::setInUse().
     *
     */
    void setInUse(bool value) override;

  private:
    friend class vtkZoomCommand;

    /** \brief ZoomSelectionWidget class constructor.
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

      /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
       *
       */
      static vtkZoomCommand *New()
      { return new vtkZoomCommand(); }

      /** \brief Implements vtkEspinaCommand::Execute.
       *
       */
      void Execute(vtkObject *, unsigned long int, void*);

      /** \brief Implements vtkEspinaCommand::setWidget();
       *
       */
      void setWidget(EspinaWidgetPtr widget)
      { m_widget = dynamic_cast<ZoomSelectionWidget *>(widget); }

    private:
      /** \brief vtkZoomCommand class private constructor.
       *
       */
      explicit vtkZoomCommand()
      : m_widget{nullptr}
      {}

      /** \brief vtkZoomCommand class private destructor.
       *
       */
      virtual ~vtkZoomCommand()
      {};

      ZoomSelectionWidget *m_widget;
  };

}// namespace ESPINA

#endif // ESPINA_ZOOM_SELECTION_WIDGET_H_
