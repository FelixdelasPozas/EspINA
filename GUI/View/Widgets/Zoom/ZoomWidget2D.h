/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ZOOMWIDGET2D_H_
#define ESPINA_ZOOMWIDGET2D_H_

#include "EspinaGUI_Export.h"

// ESPINA
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Zoom/ZoomEventHandler.h>

// VTK
#include <vtkSmartPointer.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class vtkZoomSelectionWidget;
        
        class EspinaGUI_EXPORT ZoomWidget2D
        : public EspinaWidget2D
        {
            Q_OBJECT
          public:
            /** \brief ZoomWidget2D class constructor.
             * \param[in] eventHandler handler pointer.
             *
             */
            explicit ZoomWidget2D(ZoomEventHandler *eventHandler);

            /** \brief ZoomWidget2D class virtual destructor.
             *
             */
            virtual ~ZoomWidget2D();

            virtual void setPlane(Plane plane);

            virtual void setRepresentationDepth(Nm depth)
            {};

          private slots:
            /** \brief Handle mouse movements.
             * \param[in] point global position of the event.
             * \param[in] view view of the event
             *
             */
            void onMouseMovement(QPoint point, RenderView *view);

            /** \brief Handle mouse left button presses.
             * \param[in] point global position of the event.
             * \param[in] view view of the event
             *
             */
            void onLeftMousePress(QPoint point, RenderView *view);

            /** \brief Handle mouse left button releases.
             * \param[in] point global position of the event.
             * \param[in] view view of the event
             *
             */
            void onLeftMouseRelease(QPoint point, RenderView *view);

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view);

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private:
            virtual ESPINA::GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

          private:
            ZoomEventHandler                       *m_eventHandler;
            vtkSmartPointer<vtkZoomSelectionWidget> m_widget;
            Plane                                   m_plane;
            RenderView                             *m_view;
        };

      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_ZOOMWIDGET2D_H_
