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

#ifndef ESPINA_SKELETON_WIDGET_2D_H_
#define ESPINA_SKELETON_WIDGET_2D_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/View/EventHandler.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Skeleton/vtkSkeletonWidget.h>
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>

// VTK
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
          /** \class SkeletonWidget
           * \brief Two dimensional widget for creating and modifying skeletons.
           *
           */
          class EspinaGUI_EXPORT SkeletonWidget2D
          : public EspinaWidget2D
          {
            Q_OBJECT
            public:
              enum class Mode : std::int8_t { CREATE = 0, MODIFY = 1, DELETE = 2, };

              /** \brief SkeletonWidget class constructor.
               * \brief handler handler for this widget.
               *
               */
              SkeletonWidget2D(SkeletonEventHandlerSPtr handler);

              /** \brief SkeletonWidget class virtual destructor.
               *
               */
              virtual ~SkeletonWidget2D();

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

              /** \brief Sets the operating mode of the widget.
               *
               */
              void setMode(Mode mode)
              { m_mode = mode; }

              /** \brief Stops the current operation.
               *
               */
              void stop();

              /** \brief Orders the vtkWidget to clean the statis skeleton representation.
               *
               */
              void cleanup()
              { m_widget->cleanup(); }

              virtual void setPlane(Plane plane);

              virtual void setRepresentationDepth(Nm depth);

              virtual Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

              virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override
              { return false; };

              virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
              { return false; };

              virtual void display(const GUI::Representations::FrameCSPtr &frame);

            protected:
              virtual bool isEnabled()
              { return true; };

              virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

              virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const
              { return false; };

              virtual void setSceneResolution(const NmVector3 &resolution)
              {}

            signals:
              void modified(vtkSmartPointer<vtkPolyData> polydata);

            private:
              using Track = SkeletonEventHandler::Track;

            private slots:
              void onTrackStarted(Track track, RenderView * view);
              void onTrackUpdated(Track track, RenderView *view);
              void onCursorPositionChanged(const QPoint &p, RenderView *view);
              void onStrokeEnded(RenderView *view);
              void onMousePress(Qt::MouseButtons button, const QPoint &p, RenderView *view);
              void onMouseRelease(Qt::MouseButtons button, const QPoint &p, RenderView *view);

            private:
              /** \brief Helper method to connect the handler signals.
               *
               */
              void connectSignals();

              /** \brief Helper method to disconnect the handler signals.
               *
               */
              void disconnectSignals();

              virtual void initializeImplementation(RenderView *view);

              virtual void uninitializeImplementation();

              virtual vtkAbstractWidget *vtkWidget()
              { return m_widget; }

              vtkSmartPointer<vtkSkeletonWidget> m_widget;     /** vtk widget.                                              */
              Nm                                 m_position;   /** position of the actors over the segmentations.           */
              SkeletonEventHandlerSPtr           m_handler;    /** event handler for the widget.                            */
              RenderView                        *m_view;       /** view of the widget.                                      */
              Mode                               m_mode;       /** current operation mode.                                  */
          };

          using SkeletonWidget2DSPtr = std::shared_ptr<SkeletonWidget2D>;

        } // namespace Skeleton
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_SKELETON_WIDGET_2D_H_
