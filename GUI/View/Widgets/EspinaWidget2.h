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
#include <Core/Utils/NmVector3.h>

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
        class EspinaWidget2D;
        using EspinaWidget2DSPtr = std::shared_ptr<EspinaWidget2D>;

        class EspinaWidget3D;
        using EspinaWidget3DSPtr = std::shared_ptr<EspinaWidget3D>;

        class EspinaGUI_EXPORT EspinaWidget
        : public QObject
        {
        public:
          /** \brief EspinaWidget class constructor.
           *
           */
          explicit EspinaWidget();

          /** \brief EspinaWidget class virtual destructor.
           *
           */
          virtual ~EspinaWidget();

          void initialize(RenderView *view);

          void uninitialize();

          void show();

          void hide();

          bool isEnabled();

          virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const = 0;

          virtual void setCrosshair(const NmVector3 &crosshair) {}

          virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const = 0;

          virtual void setSceneResolution(const NmVector3 &resolution) {}

        private:
          virtual void initializeImplementation(RenderView *view) = 0;

          virtual void uninitializeImplementation() = 0;

          virtual vtkAbstractWidget *vtkWidget() = 0;
        };

        class EspinaWidget2D
        : public EspinaWidget
        {
        public:
          virtual void setPlane(Plane plane) = 0;

          virtual void setRepresentationDepth(Nm depth) = 0;

          virtual EspinaWidget2DSPtr clone() = 0;

        };

        class EspinaWidget3D
        : public EspinaWidget
        {
        public:
          virtual EspinaWidget3DSPtr clone() = 0;
        };

      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_WIDGET_H
