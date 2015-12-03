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
#include "MeasureEventHandler.h"
#include <GUI/View/EventHandler.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// VTK
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt
#include <QMap>

class vtkDistanceWidget;
class vtkProperty2D;
class vtkHandleRepresentation;
class vtkDistanceRepresentation2D;
class vtkAbstractWidget;
class vtkDistanceWidget;
class vtkCamera;

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
          class EspinaGUI_EXPORT MeasureWidget
          : public EspinaWidget2D
          {
            Q_OBJECT

            class vtkDistanceCommand;

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

            virtual Representations::Managers::TemporalRepresentation2DSPtr clone();

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view);

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
