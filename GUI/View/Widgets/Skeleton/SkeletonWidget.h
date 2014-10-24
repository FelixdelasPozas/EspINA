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
#include "Core/Utils/Spatial.h"
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/EventHandler.h>
#include <Core/Utils/NmVector3.h>


// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

class vtkPolyData;

namespace ESPINA
{
  class vtkSkeletonWidget;
  class vtkSkeletonWidgetCommand;

  class EspinaGUI_EXPORT SkeletonWidget
  : public EventHandler
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /** \brief SkeletonWidget class constructor.
       *
       */
      SkeletonWidget();

      /** \brief SkeletonWidget class virtual destructor.
       *
       */
      virtual ~SkeletonWidget();

      virtual void registerView(RenderView *view);

      virtual void unregisterView(RenderView *view);

      virtual void setEnabled(bool enable);

      bool filterEvent(QEvent *e, RenderView *view) override;

      void setInUse(bool value) override;

      /** \brief Sets the minimum distance between points.
       * \param[in] value minimum distance between points.
       */
      void setTolerance(const double value);

      /** \brief Sets the spacing of the skeleton to make all the nodes centered in the voxels.
       *
       */
      void setSpacing(const NmVector3 &spacing);

      /** \brief Returns the skeleton when the operation has finished.
       *
       */
      vtkSmartPointer<vtkPolyData> getSkeleton();

      /** \brief Sets the color of the representation.
       * \param[in] color Qcolor object.
       *
       */
      void setRepresentationColor(const QColor &color);

      /** \brief Initialize the vtkWidgets with a polydata.
       * \param[in] pd VtkPolyData smartPointer.
       *
       */
      void initialize(vtkSmartPointer<vtkPolyData> pd);

    public slots:
      void changeSlice(Plane, Nm);

    private:
      friend class vtkSkeletonWidgetCommand;

      vtkSmartPointer<vtkSkeletonWidgetCommand> m_command;
      QMap<RenderView *, vtkSkeletonWidget*>    m_widgets;
      double                                    m_tolerance;
      NmVector3                                 m_spacing;
  };

  class vtkSkeletonWidgetCommand
  : public vtkEspinaCommand
  {
    public:
      vtkTypeMacro(vtkSkeletonWidgetCommand, vtkEspinaCommand);

      static vtkSkeletonWidgetCommand *New()
      { return new vtkSkeletonWidgetCommand(); }

      void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

      void setWidget(EspinaWidgetPtr widget)
      { m_widget = dynamic_cast<SkeletonWidget *>(widget); }

    private:
      /** \brief SkeletonWidgetCommand class private constructor.
       *
       */
      explicit vtkSkeletonWidgetCommand()
      : m_widget{nullptr}
      {}

      /** \brief SkeletonWidgetCommand class private destructor.
       *
       */
      virtual ~vtkSkeletonWidgetCommand()
      {};

      SkeletonWidget *m_widget;
  };

} // namespace ESPINA

#endif // ESPINA_SKELETON_TOOL_WIDGET_H_
