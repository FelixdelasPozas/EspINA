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

#ifndef ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET2D_H
#define ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET2D_H

#include <GUI/Types.h>
#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Selection.h>
#include <GUI/View/Widgets/EspinaWidget.h>

class vtkCamera;
namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace SelectionMeasure
        {
          class vtkWidget2D;

          /** \class Widget2D
           * \brief Implements 2D segmentation measure widget.
           *
           */
          class EspinaGUI_EXPORT Widget2D
          : public EspinaWidget2D
          {
            Q_OBJECT
            class Command;

          public:
            /** \brief Widget2D class constructor.
             * \param[in] selection application selection.
             *
             */
            explicit Widget2D(SelectionSPtr selection);

            /** \brief Widget2D class virtual destructor.
             *
             */
            virtual ~Widget2D()
            {};

            virtual void setPlane(Plane plane);

            virtual void setRepresentationDepth(Nm depth);

            virtual void display(const GUI::Representations::FrameCSPtr &frame)
            { setCrosshair(frame->crosshair); }

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view);

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private:
            virtual Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

            /** \brief Updates the widget according to the given crosshair.
             * \param[in] crosshar crosshair point.
             *
             */
            virtual void setCrosshair(const NmVector3 &crosshair);

            /** \brief Connects the selection objects to modify this widget when ony of them changes.
             *
             */
            void synchronizeSelectionChanges();

            /** \brief Disconnects the selection objects to modify this widget when ony of them changes.
             *
             */
            void desynchronizeSelectionChanges();

            /** \brief Updates the widget visibility depending on the current slice.
             *
             */
            void updateVisibility();

          private slots:
            /** \brief Updates the widget when the group of selected objects change.
             *
             */
            void onSelectionChanged();

            /** \brief Updates the widget size according to currently selected items.
             *
             */
            void updateSelectionMeasure();

          private:
            int                          m_index;            /** plane index                                                    */
            SelectionSPtr                m_selection;        /** application selection object.                                  */
            vtkSmartPointer<vtkWidget2D> m_widget;           /** vtk widget.                                                    */
            vtkSmartPointer<Command>     m_command;          /** vtk widget command object.                                     */
            vtkCamera                   *m_camera;           /** pointer to the camera of the view (to update on zoom changes). */
            Nm                           m_slice;            /** slice position in Nm.                                          */
            SegmentationAdapterList m_selectedSegmentations; /** groups of currently selected segmentations.                    */
          };
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET2D_H
