/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_BRUSH_SELECTOR_H
#define ESPINA_BRUSH_SELECTOR_H

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/Selectors/Selector.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Support/ViewManager.h>

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QColor>
#include <QDebug>

class vtkLookupTable;
class vtkImageResliceToColors;
class vtkImageActor;
class vtkImplicitFunction;
class ImplicitImageSource;
class vtkImageData;
class Channel;
class PickableItem;

namespace EspINA
{
  class EspinaGUI_EXPORT BrushSelector
  : public Selector
  {
    Q_OBJECT
    public:
      enum BrushMode {BRUSH, ERASER};
      enum DrawMode {CREATE, MODIFY};

      typedef QPair<vtkImplicitFunction*, Bounds> BrushShape;
      typedef QList<BrushShape> BrushShapeList;

      using Spacing = itkVolumeType::SpacingType;

    public:
      explicit BrushSelector(ViewManagerSPtr vm);
      virtual ~BrushSelector();

      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr);

      // radius of the brush in screen pixels
      void setRadius(int radius);
      int radius() const
      { return m_displayRadius; }

      QCursor cursor() const
      { return m_cursor; }

      void setBorderColor(QColor color);
      void setBrushColor(QColor color);
      void setBrushImage(QImage &image);
      QColor getBrushColor();

      /// @item is used to specify the spacing of the stroke
      void setReferenceItem(ViewItemAdapterPtr item);
      Spacing referenceSpacing() const;

      void DrawingOn(RenderView *view);
      void DrawingOff(RenderView *view, SegmentationAdapterPtr segmentation);

      BinaryMaskSPtr<unsigned char> voxelSelectionMask() const;

      void initBrush();

    signals:
      void stroke(ViewItemAdapterPtr, double, double, double, Nm, Plane);
      void stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane);

    protected slots:
      virtual BrushShape createBrushShape(ViewItemAdapterPtr item,
                                          NmVector3 center,
                                          Nm radius,
                                          Plane plane) = 0;
    private:
      void buildCursor();
      void createBrush(NmVector3 &center, QPoint pos);
      bool validStroke(NmVector3 &center);
      void startStroke(QPoint pos, RenderView *view);
      void updateStroke(QPoint pos, RenderView *view);
      void stopStroke(RenderView *view);
      void startPreview(RenderView *view);
      void updatePreview(NmVector3 center, RenderView *view);
      void stopPreview(RenderView *view);

    private:
      ViewManagerSPtr m_viewManager;
      ViewItemAdapterPtr m_referenceItem;

    protected:
      int m_displayRadius; //In screen pixels
      QColor m_borderColor;
      QColor m_brushColor;
      QImage *m_brushImage;

      bool m_tracking;
      QPoint m_lastDot;
      WorldRegion m_stroke;

      Plane m_plane;
      Nm m_radius;
      Spacing m_spacing;
      int m_viewSize[2];
      double m_LL[3], m_UR[3];
      Bounds m_pBounds;
      double m_worldSize[2];

      vtkSmartPointer<vtkLookupTable> m_lut;
      vtkSmartPointer<vtkImageData> m_preview;
      vtkSmartPointer<vtkImageActor> m_actor;
      bool m_drawing;
      SegmentationAdapterPtr m_segmentation;
      BrushShapeList m_brushes;

      static const int MAX_RADIUS = 32;
    };

} // namespace EspINA

#endif // ESPINA_BRUSH_SELECTOR_H
