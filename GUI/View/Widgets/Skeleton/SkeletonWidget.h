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
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/View/EventHandler.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Skeleton/SkeletonPointTracker.h>
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

class vtkPolyData;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Skeleton
        {
          class vtkSkeletonWidget;
          class vtkSkeletonWidgetCommand;

          /** \class SkeletonWidget
           * \brief Two dimensional widget for creating and modifying skeletons.
           *
           */
          class EspinaGUI_EXPORT SkeletonWidget
          : public EspinaWidget2D
          {
            Q_OBJECT
            public:
              enum class Status : std::int8_t { READY_TO_CREATE = 0, CREATING = 1, READY_TO_EDIT = 2, EDITING = 3 };

              /** \brief SkeletonWidget class constructor.
               * \brief handler handler for this widget.
               *
               */
              SkeletonWidget(SkeletonPointTrackerSPtr handler);

              /** \brief SkeletonWidget class virtual destructor.
               *
               */
              virtual ~SkeletonWidget();

              /** \brief Sets the minimum distance between points.
               * \param[in] value minimum distance between points.
               *
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

              virtual void setPlane(Plane plane)
              { /* plane initialized in initializeImplementation() */ };

              virtual void setRepresentationDepth(Nm depth)
              { /* representation depth initialized in initializeImplementation() */ };

              virtual Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation()
              { return std::make_shared<SkeletonWidget>(m_handler);};

              virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override
              { return false; };

              virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
              { return false; };

            protected:
              virtual bool isEnabled()
              { return true; };

              virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const
              { return false; };

              virtual void setCrosshair(const NmVector3 &crosshair)
              {}

              virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const
              { return false; };

              virtual void setSceneResolution(const NmVector3 &resolution)
              {}

            public slots:
              void changeSlice(Plane, Nm);

            signals:
              void modified(vtkSmartPointer<vtkPolyData> polydata);
              void status(SkeletonWidget::Status status);

            private:
              friend class vtkSkeletonWidgetCommand;

              virtual void initializeImplementation(RenderView *view);

              virtual void uninitializeImplementation() {};

              virtual vtkAbstractWidget *vtkWidget() {return nullptr;};

              vtkSmartPointer<vtkSkeletonWidgetCommand> m_command;
              QMap<RenderView *, vtkSkeletonWidget*>    m_widgets;
              double                                    m_tolerance;
              NmVector3                                 m_spacing;
              Plane                                     m_plane;
              SkeletonPointTrackerSPtr                  m_handler;
          };

          /** \class vtkSkeletonCommand
           * \brief Handles vtk events and passes them to the skeleton widget.
           *
           */
          class vtkSkeletonWidgetCommand
          : public vtkCommand
          {
            public:
              /** \brief VTK type macro.
               *
               */
              vtkTypeMacro(vtkSkeletonWidgetCommand, vtkCommand);

              /** \brief VTK-style New method for compatibility.
               *
               */
              static vtkSkeletonWidgetCommand *New()
              { return new vtkSkeletonWidgetCommand(); }

              void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

              /** \brief Sets the widget of the vtkCommand object.
               * \param[in] widget skeleton widget associated with this object.
               *
               */
              void setWidget(SkeletonWidget *widget)
              { m_widget = widget; }

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

              SkeletonWidget *m_widget; /** widget this vtkCommand responds to. */
          };

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_SKELETON_TOOL_WIDGET_H_
