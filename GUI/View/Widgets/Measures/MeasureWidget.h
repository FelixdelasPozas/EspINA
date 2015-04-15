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
#include <GUI/View/Widgets/EspinaWidget2.h>
#include "MeasureEventHandler.h"
#include <GUI/View/EventHandler.h>

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

class vtkProperty2D;
class vtkHandleRepresentation;
class vtkDistanceRepresentation2D;
class vtkAbstractWidget;
class vtkDistanceWidget;
class vtkMeasureWidget;
class vtkCamera;
class vtkRenderWindowInteractor;

class QEvent;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Measures
        {
          class vtkDistanceCommand;

          class EspinaGUI_EXPORT MeasureWidget
          : public EspinaWidget2D
          {
            Q_OBJECT

            class vtkDistanceCommand
            : public vtkCommand
            {
              vtkTypeMacro(vtkDistanceCommand, vtkCommand);

              virtual ~vtkDistanceCommand();

              /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
               *
               */
              static vtkDistanceCommand* New()
              { return new vtkDistanceCommand(); }

              void setDistanceWidget(vtkDistanceWidget *widget);

              virtual void Execute(vtkObject *, unsigned long int, void*);

            private:
              /** \brief Class vtkDistanceCommand class private constructor.
               *
               */
              explicit vtkDistanceCommand()
              : m_widget{nullptr}
              , m_camera{nullptr}
              {}

              vtkProperty2D *pointProperty(vtkHandleRepresentation *point) const;

              /** \brief Computes optimal tick distance for current representation
               * \param[in] length numerical value.
               *
               */
              Nm optimalTickDistance(vtkDistanceRepresentation2D *representation) const;

            private:
              vtkDistanceWidget *m_widget;
              vtkCamera         *m_camera;
            };

          public:
            /** \brief Class MeasureWidget constructor.
             *
             */
            explicit MeasureWidget(MeasureEventHandler *eventHandler);

            /** \brief MeasureWidget class destructor.
             *
             */
            virtual ~MeasureWidget();

            virtual void setPlane(Plane plane);

            virtual void setRepresentationDepth(Nm depth);

            virtual EspinaWidget2DSPtr clone();

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual void initializeImplementation();

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private slots:
            void onClear();

          private:
            MeasureEventHandler                *m_eventHandler;
            vtkSmartPointer<vtkDistanceCommand> m_command;
            vtkSmartPointer<vtkDistanceWidget>  m_widget;
          };
        }
      }
    }
  }
}// namespace ESPINA

#endif // ESPINA_MEASURE_WIDGET_H_
