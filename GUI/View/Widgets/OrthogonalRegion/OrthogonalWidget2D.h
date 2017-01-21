/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_ORTHOGONAL_WIDGET_2D_H
#define ESPINA_ORTHOGONAL_WIDGET_2D_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include "OrthogonalRepresentation.h"

// VTK
#include <vtkAbstractWidget.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>

class QColor;
namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class vtkOrthogonalWidget2D;

          /** \class OrthogonalWidget2D
           * \brief
           */
          class EspinaGUI_EXPORT OrthogonalWidget2D
          : public EspinaWidget2D
          {
            Q_OBJECT

          public:
            explicit OrthogonalWidget2D(OrthogonalRepresentationSPtr representation);

            virtual void setPlane(Plane plane) override;

            virtual void setRepresentationDepth(Nm depth) override;

            virtual bool acceptCrosshairChange(const NmVector3& crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3& resolution) const override;

            virtual bool acceptSceneBoundsChange(const Bounds& bounds) const override;

            virtual bool acceptInvalidationFrame(const Representations::FrameCSPtr frame) const;

            virtual void initializeImplementation(RenderView* view) override;

            virtual void uninitializeImplementation() override;

            virtual vtkAbstractWidget* vtkWidget() override;

          private:
            virtual void display(const Representations::FrameCSPtr& frame) override;

            virtual Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation() override;

          private slots:
            void onModeChanged(const OrthogonalRepresentation::Mode mode);

            void onResolutionChanged(const NmVector3 &resolution);

            void onBoundsChanged(const Bounds &bounds);

            void onColorChanged(const QColor &color);

            void onPatternChanged(const int pattern);

          private:
            class Command;

            OrthogonalRepresentationSPtr m_representation;

            vtkSmartPointer<vtkOrthogonalWidget2D> m_widget;
            vtkSmartPointer<Command>     m_command;

            int m_index;
            Nm  m_slice;
            Nm  m_resolution;
          };
        }
      }
    }
  }
}// namespace ESPINA

#endif // ESPINA_ORTHOGONAL_WIDGET_2D_H
