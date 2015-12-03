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
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Core/Utils/vtkVoxelContour2D.h>
#include <GUI/View/Widgets/EspinaWidget.h>

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
              /** \brief ROIWidget class constructor.
               * \param[in] roi ROI to use as source for the widget representations.
               *
               */
              explicit ROIWidget(ROISPtr roi);

              virtual void initialize(RenderView *view) override;

              virtual void uninitialize() override;

              virtual void show() override;

              virtual void hide() override;

              virtual Representations::Managers::TemporalRepresentation2DSPtr clone() override;

              virtual bool acceptCrosshairChange(const NmVector3& crosshair) const override;

              virtual bool acceptSceneResolutionChange(const NmVector3& resolution) const override;

              virtual bool acceptSceneBoundsChange(const Bounds& bounds) const override;

              virtual bool acceptInvalidationFrame(const Representations::FrameCSPtr frame) const;

              virtual void setPlane(Plane plane) override;

              virtual void setRepresentationDepth(Nm depth) override;

              virtual void display(const GUI::Representations::FrameCSPtr& frame) override;

              /** \brief Sets the color of the widget's representations.
               * \param[in] color color.
               *
               */
              void setColor(const QColor &color);

            private:
              /** \brief Returns the vtkPolyData of the current slice.
               *
               */
              vtkSmartPointer<vtkImageData> currentSlice() const;

              /** \brief Updates the representation for the current slice.
               *
               */
              void updateCurrentSlice();

            private slots:
              /** \brief Updates the representation when the ROI changes.
               *
               */
              void onROIChanged();

            private:
              ROISPtr m_ROI;    /** ROI object to represent. */
              QColor  m_color;  /** color of the representation. */

              Nm        m_depth;        /** distance from the current slice where the representations must be shown. */
              Nm        m_reslicePoint; /** current representation's reslicing position. */
              int       m_index;        /** represenation's plane index. */

              vtkSmartPointer<vtkVoxelContour2D> m_contour; /** voxel contour filter.    */
              vtkSmartPointer<vtkPolyDataMapper> m_mapper;  /** representation's mapper. */
              vtkSmartPointer<vtkActor>          m_actor;   /** representation's actor.  */

              RenderView *m_view; /** view where the representations will be shown. */
          };
        }
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_ROI_WIDGET_H_
