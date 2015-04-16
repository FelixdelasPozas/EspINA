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

#ifndef ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET3D_H
#define ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET3D_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Widgets/EspinaWidget2.h>

#include <GUI/View/Selection.h>

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
          class vtkWidget3D;

          class Widget3D
          : public EspinaWidget3D
          {
            Q_OBJECT
          public:
            explicit Widget3D(SelectionSPtr selection);

            virtual EspinaWidget3DSPtr clone();

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual void initializeImplementation(RenderView *view);

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private:
            void synchronizeSelectionChanges();

            void desynchronizeSelectionChanges();

          private slots:
            void onSelectionChanged();

            void updateSelectionMeasure();

          private:
            SelectionSPtr                m_selection;
            vtkSmartPointer<vtkWidget3D> m_widget;

            SegmentationAdapterList m_selectedSegmentations;
          };
        }
      }
    }
  }
}

#endif // ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET3D_H
