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

#ifndef _VTKPLANECONTOURREPRESENTATIONGLYPH_H_
#define _VTKPLANECONTOURREPRESENTATIONGLYPH_H_

#include <GUI/View/Widgets/Contour/vtkContourToPolygonFilter.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentation.h>
#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QColor>

class vtkProperty;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkGlyph3D;
class vtkPoints;
class vtkSphereSource;

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

          class EspinaGUI_EXPORT vtkPlaneContourRepresentationGlyph
          : public vtkPlaneContourRepresentation
          {
          public:
            // Description:
            // Instantiate this class.
            static vtkPlaneContourRepresentationGlyph *New();

            // Description:
            // Standard methods for instances of this class.
            vtkTypeMacro(vtkPlaneContourRepresentationGlyph,vtkPlaneContourRepresentation);
            void PrintSelf(ostream& os, vtkIndent indent);

            // Description:
            // This is the property used when the handle is not active
            // (the mouse is not near the handle)
            vtkGetObjectMacro(Property,vtkProperty);

            // Description:
            // This is the property used when the user is interacting
            // with the handle.
            vtkGetObjectMacro(ActiveProperty,vtkProperty);

            // Description:
            // This is the property used by the lines.
            vtkGetObjectMacro(LinesProperty,vtkProperty);

            // Description:
            // These are the methods that the widget and its representation use to
            // communicate with each other.
            virtual void SetRenderer(vtkRenderer *ren);
            virtual void BuildRepresentation();
            virtual void StartWidgetInteraction(double eventPos[2]);
            virtual void WidgetInteraction(double eventPos[2]);
            virtual int ComputeInteractionState(int X, int Y, int modified = 0);

            // Description:
            // Methods to make this class behave as a vtkProp.
            virtual void GetActors(vtkPropCollection *);
            virtual void ReleaseGraphicsResources(vtkWindow *);
            virtual int RenderOverlay(vtkViewport *viewport);
            virtual int RenderOpaqueGeometry(vtkViewport *viewport);
            virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);
            virtual int HasTranslucentPolygonalGeometry();

            // Description:
            // Get the points in this contour as a vtkPolyData.
            virtual vtkPolyData *GetContourRepresentationAsPolyData();

            // Description:
            // Controls whether the contour widget should always appear on top
            // of other actors in the scene. (In effect, this will disable OpenGL
            // Depth buffer tests while rendering the contour).
            // Default is to set it to false.
            vtkSetMacro(AlwaysOnTop, int);
            vtkGetMacro(AlwaysOnTop, int);
            vtkBooleanMacro( AlwaysOnTop, int );

            // Description:
            // Convenience method to set the line color.
            // Ideally one should use GetLinesProperty()->SetColor().
            void SetLineColor(double r, double g, double b);

            // Description:
            // Return the bounds of the representation
            virtual double *GetBounds();

            // spacing is needed for interaction calculations, together with bounds
            vtkSetVector2Macro(Spacing, double);

            double Distance2BetweenPoints(int displayPosX, int displayPosY, int node);

            // get/set polygon color
            virtual void setPolygonColor(const QColor &color);
            virtual QColor getPolygonColor() const;

          protected:
            vtkPlaneContourRepresentationGlyph();
            ~vtkPlaneContourRepresentationGlyph();

            // Render the cursor
            vtkSmartPointer<vtkActor>          Actor;
            vtkSmartPointer<vtkPolyDataMapper> Mapper;
            vtkSmartPointer<vtkGlyph3D>        Glypher;
            vtkSmartPointer<vtkActor>          ActiveActor;
            vtkSmartPointer<vtkPolyDataMapper> ActiveMapper;
            vtkSmartPointer<vtkSphereSource>   ActiveSource;
            vtkSmartPointer<vtkPolyData>       FocalData;
            vtkSmartPointer<vtkPoints>         FocalPoint;

            vtkSmartPointer<vtkPolyData>       Lines;
            vtkSmartPointer<vtkPolyDataMapper> LinesMapper;
            vtkSmartPointer<vtkActor>          LinesActor;

            // Support picking
            double LastPickPosition[3];
            double LastEventPosition[2];

            // Methods to manipulate the cursor
            void Translate(double eventPos[2]);
            void Scale(double eventPos[2]);
            void ShiftContour(double eventPos[2]);
            void ScaleContour(double eventPos[2]);

            void ComputeCentroid(double* ioCentroid);

            // Properties used to control the appearance of selected objects and
            // the manipulator in general.
            vtkProperty *Property;
            vtkProperty *ActiveProperty;
            vtkProperty *LinesProperty;
            void CreateDefaultProperties();

            // Distance between where the mouse event happens and where the
            // widget is focused - maintain this distance during interaction.
            double InteractionOffset[2];

            int AlwaysOnTop;
            double Spacing[2];

            virtual void BuildLines();

            // toggles the use of a filled polygon as part of the representation
            virtual void UseContourPolygon(bool value);

          private:
            vtkPlaneContourRepresentationGlyph(const vtkPlaneContourRepresentationGlyph&); //Not implemented
            void operator=(const vtkPlaneContourRepresentationGlyph&); //Not implemented

            vtkSmartPointer<vtkActor> m_polygon;
            vtkSmartPointer<vtkContourToPolygonFilter> m_polygonFilter;
            vtkSmartPointer<vtkPolyDataMapper> m_polygonMapper;
            bool useContourPolygon;
            QColor m_polygonColor;
          };

        } // namespace Contour
      } // namespace Widgets
    } // namespace View
  } // namespace GUI
} // namespace ESPINA

#endif // _VTKPLANECONTOURREPRESENTATIONGLYPH_H_
