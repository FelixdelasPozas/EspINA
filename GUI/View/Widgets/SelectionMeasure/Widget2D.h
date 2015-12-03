/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
