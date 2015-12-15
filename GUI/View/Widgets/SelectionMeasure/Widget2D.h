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

          class Widget2D
          : public EspinaWidget2D
          {
            Q_OBJECT
            class Command;

          public:
            explicit Widget2D(SelectionSPtr selection);

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

          private:
            virtual void setCrosshair(const NmVector3 &crosshair);

            void synchronizeSelectionChanges();

            void desynchronizeSelectionChanges();

            void updateVisibility();

          private slots:
            void onSelectionChanged();

            void updateSelectionMeasure();

          private:
            int m_index;

            SelectionSPtr                m_selection;
            vtkSmartPointer<vtkWidget2D> m_widget;
            vtkSmartPointer<Command>     m_command;
            vtkCamera                   *m_camera;

            Nm                      m_slice;
            SegmentationAdapterList m_selectedSegmentations;
          };
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET2D_H
