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


#ifndef ESPINA_BRUSH_SELECTOR_H
#define ESPINA_BRUSH_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include <GUI/Selectors/Selector.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QColor>
#include <QImage>
#include <QDebug>

class vtkLookupTable;
class vtkImageResliceToColors;
class vtkImageActor;
class vtkImplicitFunction;
class vtkImageMapToColors;
class ImplicitImageSource;
class vtkImageData;
class Channel;
class PickableItem;

namespace EspINA
{
  class RenderView;

  class EspinaGUI_EXPORT BrushSelector
  : public Selector
  {
    Q_OBJECT
    public:
      enum BrushMode {BRUSH, ERASER};
      enum DrawMode {CREATE, MODIFY};

      using BrushShape     = QPair<vtkImplicitFunction*, Bounds>;
      using BrushShapeList = QList<BrushShape>;

      using Spacing = itkVolumeType::SpacingType;

    public:
      explicit BrushSelector();
      virtual ~BrushSelector();

      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr);

      // radius of the brush in screen pixels
      void setRadius(int radius);
      int radius() const
      { return m_displayRadius; }

      QCursor cursor() const
      { return m_cursor; }

      void setBorderPaintColor(QColor color);
      void setBorderEraseColor(QColor color);
      void setBrushColor(QColor color);
      void setBrushImage(const QImage& image);
      QColor getBrushColor();
      void setBrushOpacity(int value);

      /// @item is used to specify the spacing of the stroke
      void setReferenceItem(ViewItemAdapterPtr item);
      Spacing referenceSpacing() const;

      BinaryMaskSPtr<unsigned char> voxelSelectionMask() const;

      void abortOperation();
    signals:
      void radiusChanged(int);
      void drawingModeChanged(bool);

    protected slots:
      virtual BrushShape createBrushShape(ViewItemAdapterPtr item,
                                          NmVector3 center,
                                          Nm radius,
                                          Plane plane) = 0;
      virtual void updateSliceChange();

    protected:
      void buildCursor();
      Bounds buildBrushBounds(NmVector3 center);
      void getBrushPosition(NmVector3 &center, QPoint pos);
      bool validStroke(NmVector3 &center);
      virtual void startStroke(QPoint pos, RenderView *view);
      virtual void updateStroke(QPoint pos, RenderView *view);
      virtual void stopStroke(RenderView *view);
      virtual void startPreview(RenderView *view);
      virtual void updatePreview(BrushShape shape, RenderView *view);
      virtual void stopPreview(RenderView *view);
      bool ShiftKeyIsDown();

    protected:
      ViewItemAdapterPtr m_item;

      int     m_displayRadius; //In screen pixels
      QColor  m_borderPaintColor;
      QColor  m_borderEraseColor;
      QColor  m_brushColor;
      int     m_brushOpacity;
      QImage *m_brushImage;

      QPoint m_lastDot;

      Plane     m_plane;
      Nm        m_radius;
      Spacing   m_spacing;
      NmVector3 m_origin;
      int       m_viewSize[2];
      double    m_LL[3], m_UR[3];
      Bounds    m_pBounds;
      double    m_worldSize[2];

      vtkSmartPointer<vtkLookupTable> m_lut;
      vtkSmartPointer<vtkImageData>   m_preview;
      vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
      vtkSmartPointer<vtkImageActor>  m_actor;
      Bounds                          m_previewBounds;
      bool                            m_drawing;
      BrushShapeList                  m_brushes;
      NmVector3                       m_lastUdpdatePoint;
      Bounds                          m_lastUpdateBounds;
      bool                            m_tracking;
      RenderView                     *m_previewView;

      static const int MAX_RADIUS = 30;
    };

  using BrushSelectorPtr  = BrushSelector *;
  using BrushSelectorSPtr = std::shared_ptr<BrushSelector>;

} // namespace EspINA

#endif // ESPINA_BRUSH_SELECTOR_H
