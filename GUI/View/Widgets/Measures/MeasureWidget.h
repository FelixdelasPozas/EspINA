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
    /** brief Class MeasureWidget destructor.
     *
     */
    explicit MeasureWidget();

    /** brief MeasureWidget class destructor.
     *
     */
    virtual ~MeasureWidget();

    /** brief Implements EspinaWidget::registerView()
     *
     */
    virtual void registerView(RenderView *view);

    /** brief Implements EspinaWidget::unregisterView()
     *
     */
    virtual void unregisterView(RenderView *view);

    /** brief Implements EspinaWidget::setEnabled.
     *
     */
    virtual void setEnabled(bool enable);

    /** brief Overrides EventHandler::filterEvent.
     *
     */
    bool filterEvent(QEvent *e, RenderView *view) override;

    /** brief Overrides EventHandler::setInUse()
     *
     */
    void setInUse(bool value) override;

  private:
    friend class vtkDistanceCommand;

    /** brief Computes optimal tick distance in the ruler given the length.
     * \param[in] length, numerical value.
     *
     */
    double ComputeRulerTickDistance(double lenght);

    vtkSmartPointer<vtkDistanceCommand>          m_command;
    QMap<vtkDistanceWidget *, QList<vtkCamera*>> m_cameras;
    QMap<RenderView *, vtkDistanceWidget *>      m_widgets;

  };

  class vtkDistanceCommand
  : public vtkCommand
  {
    vtkTypeMacro(vtkDistanceCommand, vtkCommand);

    /** brief VTK-style New() constructor, required for using vtkSmartPointer.
     *
     */
    static vtkDistanceCommand* New()
    { return new vtkDistanceCommand(); }

    /** brief Implements vtkEspinaCommand::Execute
     *
     */
    virtual void Execute(vtkObject *, unsigned long int, void*);

    /** brief Implements vtkEspinaCommand::setWidget
     *
     */
    void setWidget(EspinaWidgetPtr widget)
    { m_widget = dynamic_cast<MeasureWidget *>(widget); }

    private:
     /** brief Class vtkDistanceCommand class private constructor.
      *
      */
     explicit vtkDistanceCommand()
     : m_widget{nullptr}
     {}

     /** brief Class vtkDistanceCommand class private destructor.
      *
      */
     virtual ~vtkDistanceCommand()
     {}

     ESPINA::MeasureWidget *m_widget;
  };

}// namespace ESPINA

#endif // ESPINA_MEASURE_WIDGET_H_
