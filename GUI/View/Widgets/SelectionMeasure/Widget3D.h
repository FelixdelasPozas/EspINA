/*

 Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET3D_H
#define ESPINA_GUI_VIEW_WIDGETS_SELECTIONMEASURE_WIDGET3D_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/View/Selection.h>
#include <GUI/View/Widgets/EspinaWidget.h>

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

          /** \class Widget3D
           * \brief Implements 3D segmentation measure widget.
           *
           */
          class EspinaGUI_EXPORT Widget3D
          : public EspinaWidget3D
          {
            Q_OBJECT
          public:
            explicit Widget3D(SelectionSPtr selection);

            virtual ~Widget3D();

          protected:
            virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

            virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

            virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView *view);

            virtual void uninitializeImplementation();

            virtual vtkAbstractWidget *vtkWidget();

          private:
            virtual Representations::Managers::TemporalRepresentation3DSPtr cloneImplementation();

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
