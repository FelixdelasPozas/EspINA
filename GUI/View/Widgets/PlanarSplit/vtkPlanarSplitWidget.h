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

#ifndef VTKPLANARSPLITWIDGET_H_
#define VTKPLANARSPLITWIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/Spatial.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

class vtkHandleWidget;
class vtkWidgetRepresentation;
class vtkPoints;
class vtkLineSource;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class vtkPlanarSplitRepresentation2D;
        class vtkPlanarSplitWidgetCallback;

        /** \class vtkPlanarSplitWidget
         * \brief Implements planar widget for split tool in VTK.
         *
         */
        class EspinaGUI_EXPORT vtkPlanarSplitWidget
        : public vtkAbstractWidget
        {
        public:
          /** \brief Creates a new instance.
           *
           */
          static vtkPlanarSplitWidget *New();

          vtkTypeMacro(vtkPlanarSplitWidget,vtkAbstractWidget);

          virtual void SetEnabled(int) override;

          /** \brief Specify an instance of vtkWidgetRepresentation used to represent this
           *          widget in the scene. The representation is a subclass of vtkProp so
           *          it can be added to the renderer independent of the widget.
           *
           */
          void SetRepresentation(vtkPlanarSplitRepresentation2D *r)
          {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}

          virtual void SetProcessEvents(int) override;

          void CreateDefaultRepresentation();

          /** \brief Enum defining the state of the widget. By default the widget is in Start mode,
           * and expects to be interactively placed. While placing the points the widget
           * transitions to Define state. Once placed, the widget enters the Manipulate state.
           *
           */
          //BTX
          enum {Start=0,Define,Manipulate};
          //ETX

          /** \brief Set the state of the widget. If the state is set to "Manipulate" then it
           * is assumed that the widget and its representation will be initialized
           * programmatically and is not interactively placed. Initially the widget
           * state is set to "Start" which means nothing will appear and the user
           * must interactively place the widget with repeated mouse selections. Set
           * the state to "Start" if you want interactive placement. Generally state
           * changes must be followed by a Render() for things to visually take
           * effect.
           *
           */
          virtual void SetWidgetStateToStart();
          virtual void SetWidgetStateToManipulate();

          /** \brief Return the current widget state.
           *
           */
          virtual int GetWidgetState()
          {return this->WidgetState;}

          /** \brief Set the widget segment points.
           * \param[in] points
           *
           */
          virtual void setPoints(vtkSmartPointer<vtkPoints> points);

          /** \brief Returns the points of the segment.
           *
           */
          vtkSmartPointer<vtkPoints> getPoints();

          /** \brief Sets widget orientation.
           * \param[in] plane orientation plane.
           *
           */
          virtual void setPlane(Plane plane);

          /** \brief Sets the distance of the widget over the rest of the view's representations.
           * \param[in] depth view's wigets' depth value.
           *
           */
          virtual void setRepresentationDepth(const Nm shift);

          /** \brief Returns the widget's plane.
           *
           */
          virtual Plane getPlane() const
          { return m_plane; }

          virtual void PrintSelf(ostream &os, vtkIndent indent) override;

          /** \brief Disables the widget.
           *
           */
          virtual void disableWidget();

          /** \brief Sets the segmentations bounds to draw the widget.
           * \param[in] bounds pointer to a vector of six double values.
           *
           */
          virtual void setSegmentationBounds(double *bounds);

          /** \brief Sets the slice of the widget.
           * \param[in] slice slice of the view of the widget.
           *
           */
          virtual void setSlice(double slice);

          /** \brief Returns the current slice point.
           *
           */
          Nm slice() const
          { return m_slice; }

        protected:
          /** \brief vtkPlanarSplitWidget class constructor.
           *
           */
          vtkPlanarSplitWidget();

          /** \brief vtkPlanarSplitWidget class destructor.
           *
           */
          virtual ~vtkPlanarSplitWidget();

          // The state of the widget
          int    WidgetState;             /** current state of the widget.          */
          int    CurrentHandle;           /** index of current interaction handle.  */
          double m_segmentationBounds[6]; /** bounds of the segmentation to split.  */
          Plane  m_plane;                 /** representation's orientation plane.   */
          Nm     m_slice;                 /** representation's current slice point. */
          Nm     m_depth;                 /** representation's depth in the view.   */

          /** \brief Callback when adding a point.
           * \param[in] w calling widget.
           *
           */
          static void AddPointAction(vtkAbstractWidget *w);

          /** \brief Callback when moving a point.
           * \param[in] w calling widget.
           *
           */
          static void MoveAction(vtkAbstractWidget *w);

          /** \brief Callback when ending a interaction.
           * \param[in] w calling widget.
           *
           */
          static void EndSelectAction(vtkAbstractWidget *w);

          vtkHandleWidget              *m_point1Widget;               /** handle 1 widget.       */
          vtkHandleWidget              *m_point2Widget;               /** handle 2 widget.       */
          vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback1; /** callback for handle 1. */
          vtkPlanarSplitWidgetCallback *m_planarSplitWidgetCallback2; /** callback for handle 2. */

          /** \brief Method invoked when the handles at the
           * end points of the widget are manipulated
           *
           */
          void StartHandleInteraction(int handleNum);
          void HandleInteraction(int handleNum);
          void StopHandleInteraction(int handleNum);

          friend class vtkPlanarSplitWidgetCallback;

          bool m_permanentlyDisabled;
        };

        /** \class vtkPlanarSplitWidgetCallback
         * \brief The vtkPlanarSplitWidget class observes it's two handles.
         *        Here we create the command/observer classes to respond to the handle widgets.
         *
         */
        class EspinaGUI_EXPORT vtkPlanarSplitWidgetCallback
        : public vtkCommand
        {
          public:
            /** \brief Creates a new instance of vtkPlanarSplitWidgetCallback class.
             *
             */
            static vtkPlanarSplitWidgetCallback *New()
            {
              return new vtkPlanarSplitWidgetCallback;
            }

            vtkTypeMacro(vtkPlanarSplitWidgetCallback,vtkCommand);

            virtual void Execute(vtkObject*, unsigned long eventId, void*)
            {
              switch (eventId)
              {
                case vtkCommand::StartInteractionEvent:
                  this->m_widget->StartHandleInteraction(this->m_handleNumber);
                  break;
                case vtkCommand::InteractionEvent:
                  this->m_widget->HandleInteraction(this->m_handleNumber);
                  break;
                case vtkCommand::EndInteractionEvent:
                  this->m_widget->StopHandleInteraction(this->m_handleNumber);
                  break;
              }
            }

            int                   m_handleNumber; /** handler number.       */
            vtkPlanarSplitWidget *m_widget;       /** widget to respond to. */
        };

      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif /* VTKPLANARSPLITWIDGET_H_ */
