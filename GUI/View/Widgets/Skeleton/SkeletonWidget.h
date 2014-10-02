/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_TOOL_WIDGET_H_
#define ESPINA_SKELETON_TOOL_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/EventHandler.h>

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

namespace ESPINA
{
  class vtkSkeletonWidget;
  class vtkSkeletonWidgetCommand;

  class EspinaGUI_EXPORT SkeletonWidget
  : public EspinaWidget
  , public EventHandler
  {
    public:
      /** \brief SkeletonWidget class constructor.
       *
       */
      SkeletonWidget();

      /** \brief SkeletonWidget class virtual destructor.
       *
       */
      virtual ~SkeletonWidget();

      /** \brief Implements EspinaWidget::registerView()
       *
       */
      virtual void registerView(RenderView *view);

      /** \brief Implements EspinaWidget::unregisterView()
       *
       */
      virtual void unregisterView(RenderView *view);

      /** \brief Implements EspinaWidget::setEnabled.
       *
       */
      virtual void setEnabled(bool enable);

      /** \brief Overrides EventHandler::filterEvent.
       *
       */
      bool filterEvent(QEvent *e, RenderView *view) override;

      /** \brief Overrides EventHandler::setInUse()
       *
       */
      void setInUse(bool value) override;

      /** \brief Sets the minimum distance between points.
       * \param[in] value minimum distance between points.
       */
      void setTolerance(int value);

    private:
      friend class vtkSkeletonWidgetCommand;

      vtkSmartPointer<vtkSkeletonWidgetCommand> m_command;
      QMap<RenderView *, vtkSkeletonWidget *>   m_widgets;
      int                                       m_tolerance;
  };

  class vtkSkeletonWidgetCommand
  : public vtkEspinaCommand
  {
    vtkTypeMacro(vtkSkeletonWidgetCommand, vtkCommand);

    /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
     *
     */
    static vtkSkeletonWidgetCommand* New()
    { return new vtkSkeletonWidgetCommand(); }

    /** \brief Implements vtkEspinaCommand::Execute
     *
     */
    virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

    /** \brief Implements vtkEspinaCommand::setWidget
     *
     */
    void setWidget(EspinaWidgetPtr widget)
    { m_widget = dynamic_cast<SkeletonWidget *>(widget); }

    private:
     /** \brief Class vtkDistanceCommand class private constructor.
      *
      */
     explicit vtkSkeletonWidgetCommand()
     : m_widget{nullptr}
     {}

     /** \brief Class vtkDistanceCommand class private destructor.
      *
      */
     virtual ~vtkSkeletonWidgetCommand()
     {}

     SkeletonWidget *m_widget;
  };

} // namespace ESPINA

#endif // ESPINA_SKELETON_TOOL_WIDGET_H_
