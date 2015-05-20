/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

          void initializeWidget(RenderView *view);

          void uninitializeWidget();

          void showWidget();

          void hideWidget();

          bool isWidgetEnabled();

        private:
          virtual void initializeImplementation(RenderView *view) = 0;

          virtual void uninitializeImplementation() = 0;

          virtual vtkAbstractWidget *vtkWidget() = 0;
        };

        class EspinaWidget2D
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

          virtual bool isEnabled() override
          { return isWidgetEnabled(); }
        };

        class EspinaWidget3D
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

          virtual bool isEnabled() override
          { return isWidgetEnabled(); }
        };

      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_WIDGET_H
