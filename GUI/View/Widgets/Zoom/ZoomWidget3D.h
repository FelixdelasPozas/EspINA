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

#ifndef ESPINA_ZOOMWIDGET3D_H_
#define ESPINA_ZOOMWIDGET3D_H_

// ESPINA
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Zoom/ZoomEventHandler.h>

// VTK
#include <vtkSmartPointer.h>

using namespace ESPINA::GUI::Representations::Managers;

namespace ESPINA
{
  class RenderView;

  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class vtkZoomSelectionWidget;
        
        class EspinaGUI_EXPORT ZoomWidget3D
        : public EspinaWidget3D
        {
            Q_OBJECT
          public:
            /** \brief ZoomWidget3D class constructor.
             * \param[in] eventHandler handler pointer.
             *
             */
            explicit ZoomWidget3D(ZoomEventHandler *eventHandler);

            /** \brief ZoomWidget3D class virtual destructor.
             *
             */
            virtual ~ZoomWidget3D();

            virtual TemporalRepresentation3DSPtr clone();

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

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view);

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private:
            ZoomEventHandler                       *m_eventHandler;
            vtkSmartPointer<vtkZoomSelectionWidget> m_widget;
            RenderView                             *m_view;
        };
      
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_ZOOMWIDGET3D_H_
