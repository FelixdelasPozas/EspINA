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

#ifndef ESPINA_ROI_WIDGET_H_
#define ESPINA_ROI_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"
#include <GUI/View/Widgets/EspinaWidget2.h>

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Core/Utils/vtkVoxelContour2D.h>

// VTK
#include <vtkActor.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>

// Qt
#include <QObject>
#include <QColor>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace ROI
        {
          class EspinaGUI_EXPORT ROIWidget
          : public Representations::Managers::TemporalRepresentation2D
          {
            Q_OBJECT

          public:
            explicit ROIWidget(ROISPtr roi);

            virtual void initialize(RenderView *view);

            virtual void uninitialize();

            virtual void show();

            virtual void hide();

            virtual bool isEnabled();

            virtual Representations::Managers::TemporalRepresentation2DSPtr clone() override;

            virtual bool acceptCrosshairChange(const NmVector3& crosshair) const override;

            virtual bool acceptSceneResolutionChange(const NmVector3& resolution) const override;

            virtual void setPlane(Plane plane) override;

            virtual void setRepresentationDepth(Nm depth) override;

            void setColor(const QColor &color);

          private:
            virtual void setCrosshair(const NmVector3 &crosshair) override;

            vtkSmartPointer<vtkImageData> currentSlice() const;

            void updateCurrentSlice();

            Nm normalCoordinate(const NmVector3 &value) const;

          private slots:
            void onROIChanged();

          private:
            ROISPtr m_ROI;
            QColor  m_color;

            vtkSmartPointer<vtkVoxelContour2D> m_contour;
            vtkSmartPointer<vtkPolyDataMapper> m_mapper;
            vtkSmartPointer<vtkActor>          m_actor;

            int   m_normalIndex;
            Nm    m_depth;

            Nm     m_reslicePosition;
            bool   m_isEnabled;
            RenderView *m_view;
          };
        }
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_ROI_WIDGET_H_
