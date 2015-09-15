/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_VIEW_WIDGETS_CONTOUR_CONTOURWIDGET2D_H_
#define GUI_VIEW_WIDGETS_CONTOUR_CONTOURWIDGET2D_H_

// ESPINA
#include <GUI/EventHandlers/MaskPainter.h>
#include <GUI/EventHandlers/ContourPainter.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace Contour
        {
          class vtkPlaneContourWidget;

          class ContourWidget2D
          : public EspinaWidget2D
          {
              Q_OBJECT

            public:
              struct ContourData
              {
                Nm                           slice;
                Plane                        plane;
                DrawingMode                  mode;
                vtkSmartPointer<vtkPolyData> polyData;

                ContourData(Nm actual, Nm position, Plane plane, DrawingMode mode, vtkSmartPointer<vtkPolyData> contour) : slice{position}, plane{plane}, mode{mode}, polyData{contour} {};
                ContourData() : slice{0}, plane{Plane::XY}, mode{DrawingMode::PAINTING}, polyData{} { polyData = nullptr; };
                ~ContourData() { polyData = nullptr; }
              };

              using Contour = struct ContourData;

            public:
              explicit ContourWidget2D(ContourPainterSPtr handler);
              virtual ~ContourWidget2D();

              virtual void setPlane(Plane plane);

              virtual void setRepresentationDepth(Nm depth);

              virtual Representations::Managers::TemporalRepresentation2DSPtr clone();

              /** \brief Resets all contours in all planes without rasterizing and uses the given contour list as initial data.
               * \param[in] contour initial contour.
               *
               */
              void initialize(Contour contour);

            signals:
              void contour(BinaryMaskSPtr<unsigned char> mask);

            protected slots:
              /** \brief Resets all contours in all planes without rasterizing.
               *
               */
              void initialize();

              /** \brief Configures the widget.
               *
               */
              void configure(Nm distance, QColor color, NmVector3 spacing);

              /** \brief Sets the mode of the widget.
               * \param[in] mode Brush mode.
               */
              void setDrawingMode(DrawingMode mode);

              /** \brief Used by the vtkAbstractWidget to signal the start of a new contour.
               *
               */
              void rasterize();

            protected:
              virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const;

              virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const;

            private:
              virtual void initializeImplementation(RenderView *view);

              virtual void uninitializeImplementation();

              virtual vtkAbstractWidget *vtkWidget();

              virtual void setCrosshair(const NmVector3 &crosshair);

              void notifyContourEnd();

            private:
              friend class vtkPlaneContourWidget;

              ContourPainterSPtr m_handler;
              Contour            m_storedContour;
              vtkSmartPointer<vtkPlaneContourWidget> m_widget;

              int       m_slice;
              int       m_index;
              NmVector3 m_spacing;
          };

        } // namespace Contour
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_CONTOUR_CONTOURWIDGET2D_H_
