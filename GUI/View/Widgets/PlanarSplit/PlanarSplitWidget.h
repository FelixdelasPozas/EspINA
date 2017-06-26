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

#ifndef PLANARSPLITWIDGET_H_
#define PLANARSPLITWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Spatial.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// Qt
#include <QMap>

class vtkAbstractWidget;
class vtkPoints;
class vtkPlane;
class vtkAbstractWidget;
class vtkImageStencilSource;
class vtkImplicitPlaneWidget2;
class vtkAlgorithmOutput;
class vtkImageStencilData;

namespace ESPINA
{
  namespace GUI
  {
    class View2D;

    namespace View
    {
      namespace Widgets
      {
        class PlanarSplitEventHandler;
        using PlanarSplitEventHandlerPtr = PlanarSplitEventHandler *;

        class vtkSplitCommand;

        class PlanarSplitWidget;
        using PlanarSplitWidgetPtr  = PlanarSplitWidget *;
        using PlanarSplitWidgetSPtr = std::shared_ptr<PlanarSplitWidget>;

        class EspinaGUI_EXPORT PlanarSplitWidget
        {
          public:
            /** \brief PlanarSplitWidget class constructor.
             * \param[in] handler PlanarSplitEventHandler raw pointer.
             *
             */
            explicit PlanarSplitWidget(PlanarSplitEventHandler *handler);

            /** \brief PlanarSplitWidget class virtual destructor.
             *
             */
            virtual ~PlanarSplitWidget()
            {};

            /** \brief Disables the widget permanently.
             *
             */
            virtual void disableWidget() = 0;

            /** \brief Set plane points.
             * \param[in] points
             *
             */
            virtual void setPlanePoints(vtkSmartPointer<vtkPoints> points) = 0;

            /** \brief Get plane points.
             *
             */
            virtual vtkSmartPointer<vtkPoints> getPlanePoints() const = 0;

            /** \brief Returns the vtkPlane defined in the tool.
             * \param[in] spacing
             */
            virtual vtkSmartPointer<vtkPlane> getImplicitPlane(const NmVector3 &spacing) const = 0;

            /** \brief Sets the bounds of the segmentation to be splitted.
             * \param[in] bounds bounds of the segmentation.
             *
             */
            virtual void setSegmentationBounds(const Bounds &bounds) = 0;

            /** \brief Returns true if the defined plane is valid.
             *
             */
            virtual bool planeIsValid() const = 0;

          protected:
            PlanarSplitEventHandler *m_handler;

            friend class vtkSplitCommand;
        };

        /** \class vtkSplitCommand
         * \brief Handles VTK events regarding the VTK widgets.
         *
         */
        class EspinaGUI_EXPORT vtkSplitCommand
        : public vtkCommand
        {
          public:
            vtkTypeMacro(vtkSplitCommand, vtkCommand)
            ;

            /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
             *
             */
            static vtkSplitCommand *New()
            {
              return new vtkSplitCommand();
            }

            /** \brief Implements vtkEspinaCommand::Execute().
             *
             */
            virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData);

            /** \brief Implements vtkEspinaCommand::setWidget()
             *
             */
            virtual void setWidget(PlanarSplitWidgetPtr widget)
            {
              m_widget = widget;
            }

            virtual void setHandler(PlanarSplitEventHandlerPtr handler)
            {
              m_handler = handler;
            }

          private:
            /** \brief vtkSplitCommand private class constructor.
             *
             */
            vtkSplitCommand()
            : m_widget {nullptr}
            , m_handler{nullptr}
            {};

            /** \brief vtkSplitCommand class private destructor.
             *
             */
            virtual ~vtkSplitCommand()
            {};

            PlanarSplitWidgetPtr m_widget;
            PlanarSplitEventHandlerPtr m_handler;
        };
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif /* PLANARSPLITWIDGET_H_ */
