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

#include <GUI/EspinaGUI_Export.h>

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

          /** \class ContourWidget2D
           * \brief EspinaWidget2D for contour tool.
           *
           */
          class EspinaGUI_EXPORT ContourWidget2D
          : public EspinaWidget2D
          {
              Q_OBJECT
            public:
              /** \struct ContourData
               * \brief Data of a contour.
               *
               */
              struct ContourData
              {
                Nm                           slice;    /** slice of the contour.     */
                Plane                        plane;    /** plane of the contour.     */
                DrawingMode                  mode;     /** contour mode (draw/erase) */
                vtkSmartPointer<vtkPolyData> polyData; /** node data as vtkPolyData. */

                ContourData(Nm actual, Nm position, Plane plane, DrawingMode mode, vtkSmartPointer<vtkPolyData> contour) : slice{position}, plane{plane}, mode{mode}, polyData{contour} {};
                ContourData() : slice{0}, plane{Plane::XY}, mode{DrawingMode::PAINTING}, polyData{} { polyData = nullptr; };
                ~ContourData() { polyData = nullptr; }
              };

              using Contour = struct ContourData;

            public:
              /** \brief ContourWidget2D class cosntructor.
               * \param[in] hander Contour tool event handler.
               *
               */
              explicit ContourWidget2D(ContourPainterSPtr handler);

              /** \brief ContourWidget2D class virtual destructor.
               *
               */
              virtual ~ContourWidget2D();

              virtual void setPlane(Plane plane);

              virtual void setRepresentationDepth(Nm depth);

              /** \brief Resets all contours in all planes without rasterizing and uses the given contour list as initial data.
               * \param[in] contour initial contour.
               *
               */
              void initializeContour(Contour contour);

            signals:
              void contour(BinaryMaskSPtr<unsigned char> mask);

            protected slots:
              /** \brief Resets all contours in all planes without rasterizing.
               *
               */
              void resetContours();

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

              virtual bool acceptSceneBoundsChange(const Bounds &bounds) const;

              virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const;

              virtual void display(const GUI::Representations::FrameCSPtr &frame) override;

            private:
              virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation();

              virtual void initializeImplementation(RenderView *view);

              virtual void uninitializeImplementation();

              virtual vtkAbstractWidget *vtkWidget();

              /** \brief Updates the widget with the representation associated with the given crosshair (if any) or stores the representatio of the previous slice.
               * \param[in] crosshair crosshair point.
               *
               */
              virtual void setCrosshair(const NmVector3 &crosshair);

              /** \brief Rastizes the current contour on termination.
               *
               */
              void notifyContourEnd();

            private:
              friend class vtkPlaneContourWidget;

              ContourPainterSPtr                     m_handler;       /** contour tool event handler.      */
              Contour                                m_storedContour; /** data of stored contour (if any). */
              vtkSmartPointer<vtkPlaneContourWidget> m_widget;        /** vtk widget.                      */
              Nm                                     m_slice;         /** current slice position in Nm.    */
              int                                    m_index;         /** view index of widget.            */
              NmVector3                              m_spacing;       /** spacing of the view.             */
          };

        } // namespace Contour
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_VIEW_WIDGETS_CONTOUR_CONTOURWIDGET2D_H_
