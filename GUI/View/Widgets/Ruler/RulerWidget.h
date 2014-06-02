/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_RULER_WIDGET_H_
#define ESPINA_RULER_WIDGET_H_

#include <EspinaGUI_Export.h>

// EspINA
#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// VTK
#include <vtkSmartPointer.h>

class vtkCubeAxesActor2D;
class vtkRenderWindowInteractor;
class vtkAbstractWidget;

namespace EspINA
{
  class vtkRulerCommand;

  class EspinaGUI_EXPORT RulerWidget
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /* \brief RulerWidget class constructor.
       *
       */
      explicit RulerWidget();

      /* \brief RulerWidget class destructor.
       *
       */
      virtual ~RulerWidget();

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

      /* \brief Sets widgets' bounds.
       * \param[in] bounds Bounds of the selection.
       *
       */
      void setBounds(Bounds bounds);

    private slots:
      void sliceChanged(Plane, Nm);

    private:
      friend class vtkRulerCommand;

      vtkSmartPointer<vtkRulerCommand> m_command;
      QMap<RenderView *, vtkAbstractWidget *> m_views;
  };

  class vtkRulerCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkRulerCommand, vtkEspinaCommand);

      /* \brief VTK-style New() constructor, required for using vtkSmartPointer.
       *
       */
      static vtkRulerCommand *New()
      { return new vtkRulerCommand(); }

      /* \brief Implements vtkEspinaCommand::Execute.
       *
       */
      void Execute(vtkObject *, unsigned long int, void*);

      /* \brief Implements vtkEspinaCommand::setWidget();
       *
       */
      void setWidget(EspinaWidgetPtr widget)
      { m_widget = dynamic_cast<RulerWidget *>(widget); }

    private:
      /* \brief RulerCommand class private constructor.
       *
       */
      explicit vtkRulerCommand()
      : m_widget{nullptr}
      {}

      /* \brief RulerCommand class private destructor.
       *
       */
      virtual ~vtkRulerCommand()
      {};

      RulerWidget *m_widget;
  };

} // namespace EspINA
#endif // ESPINA_RULER_WIDGET_H_
