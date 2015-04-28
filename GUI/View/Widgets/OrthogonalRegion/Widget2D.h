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


#ifndef ESPINA_RECTANGULAR_REGION_H
#define ESPINA_RECTANGULAR_REGION_H

#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget2.h>
#include "Representation.h"

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
          class vtkWidget2D;

          class Widget2D
          : public EspinaWidget2D
          {
            Q_OBJECT

          public:
            explicit Widget2D(Representation &representation);

            virtual void setPlane(Plane plane) override;

            virtual void setRepresentationDepth(Nm depth) override;

            virtual EspinaWidget2DSPtr clone() override;

          protected:
            virtual bool acceptCrosshairChange(const NmVector3& crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3& resolution) const override;

            virtual void initializeImplementation(RenderView* view) override;

            virtual void uninitializeImplementation() override;

            virtual vtkAbstractWidget* vtkWidget() override;

          private:
            virtual void setCrosshair(const NmVector3& crosshair) override;

          private slots:
            void onModeChanged(const Representation::Mode mode);

            void onResolutionChanged(const NmVector3 &resolution);

            void onBoundsChanged(const Bounds &bounds);

            void onColorChanged(const QColor &color);

            void onPatternChanged(const int pattern);

          private:
            class Command;

            Representation &m_representation;

            vtkSmartPointer<vtkWidget2D> m_widget;
            vtkSmartPointer<Command>     m_command;

            int m_index;
            Nm  m_slice;
          };
        }
      }
    }
  }
}// namespace ESPINA

#endif // ESPINA_RECTANGULAR_REGION_H
