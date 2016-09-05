/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_GUI_WIDGET_H
#define ESPINA_GUI_WIDGET_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Representations/Managers/TemporalManager.h>

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// C++
#include <memory>

class vtkAbstractWidget;
namespace ESPINA
{
  class RenderView;
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        /** \class EspinaWidget
         * \brief Super class for Espina interactive widgets.
         *
         */
        class EspinaGUI_EXPORT EspinaWidget
        {
          public:
            /** \brief Default constructor.
             *
             */
            explicit EspinaWidget();

            /** \brief Virtual destructor.
             *
             */
            virtual ~EspinaWidget();

            /** \brief Initializes the widget for the given view.
             * \param[in] view Espina view.
             *
             */
            void initializeWidget(RenderView *view);

            /** \brief De-initializes the widget.
             *
             */
            void uninitializeWidget();

            /** \brief Shows the widget on the configured view.
             *
             */
            void showWidget();

            /** \brief Hides the widget from the configured view.
             *
             */
            void hideWidget();

            /** \brief Returns true if the widget is shown on the configured view and false otherwise.
             *
             */
            bool isWidgetEnabled();

          private:
            /** \brief Private implementation of initialization for the widget subclass.
             *
             */
            virtual void initializeImplementation(RenderView *view) = 0;

            /** \brief Private implementation of de-initialization for the widget subclass.
             *
             */
            virtual void uninitializeImplementation() = 0;

            /** \brief Returns the vtkWidget of the EspinaWidget.
             *
             */
            virtual vtkAbstractWidget *vtkWidget() = 0;
        };

        class EspinaGUI_EXPORT EspinaWidget2D
        : public Representations::Managers::TemporalRepresentation2D
        , public EspinaWidget
        {
          public:
            virtual void initialize(RenderView *view) override
            { initializeWidget(view); }

            virtual void uninitialize() override
            { uninitializeWidget(); }

            virtual void show() override
            { showWidget(); }

            virtual void hide() override
            { hideWidget(); }
        };

        class EspinaGUI_EXPORT EspinaWidget3D
        : public Representations::Managers::TemporalRepresentation3D
        , public EspinaWidget
        {
          public:
            virtual void initialize(RenderView *view) override
            { initializeWidget(view); }

            virtual void uninitialize() override
            { uninitializeWidget(); }

            virtual void show() override
            { showWidget(); }

            virtual void hide() override
            { hideWidget(); }
        };

      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_WIDGET_H
