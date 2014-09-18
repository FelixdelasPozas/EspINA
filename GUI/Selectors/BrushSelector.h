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

// ESPINA
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

namespace ESPINA
{
  class RenderView;

  class EspinaGUI_EXPORT BrushSelector
  : public Selector
  {
    Q_OBJECT
    public:
      enum class BrushMode: std::int8_t {BRUSH = 0, ERASER = 1};
      enum class DrawMode: std::int8_t {CREATE = 0, MODIFY = 1};

      using BrushShape     = QPair<vtkSmartPointer<vtkImplicitFunction>, Bounds>;
      using BrushShapeList = QList<BrushShape>;

      using Spacing = itkVolumeType::SpacingType;

    public:
      /* \brief BrushSelector class constructor.
       *
       */
      explicit BrushSelector();

      /* \brief BrushSelector class virtual destructor.
       *
       */
      virtual ~BrushSelector();

      /* \brief Overrides EventHandler::filterEvent().
       *
       */
      virtual bool filterEvent(QEvent* e, RenderView* view = nullptr) override;

      /* \brief Sets the mode of the brush to erase/draw depending on the parameter.
       * \param[in] value, true to set to erase, sets to draw otherwise.
       *
       */
      void setEraseMode(bool value);

      /* \brief Sets the radius of the brush in screen pixels.
       * \param[in] radius, radius of the brush.
       */
      void setRadius(int radius);

      /* \brief Returns the radius of the brush.
       *
       */
      int radius() const
      { return m_displayRadius; }

      /* \brief Overrides EventHandler::cursor() const.
       *
       */
      QCursor cursor() const override
      { return m_cursor; }

      /* \brief Sets the border of the cursor for the paint mode.
       * \param[in] color, new color for the border.
       *
       */
      void setBorderPaintColor(QColor color);

      /* \brief Sets the border of the cursor for the erase mode.
       * \param[in] color, new color for the border.
       *
       */
      void setBorderEraseColor(QColor color);

      /* \brief Sets the cursor color().
       * \param[in] color, new color.
       *
       */
      void setBrushColor(QColor color);

      /* \brief Sets the cursor image().
       * \param[in] image, QImage.
       *
       */
      void setBrushImage(const QImage& image);

      /* \brief Returns the brush color.
       *
       */
      QColor getBrushColor();

      /* \brief Sets the opacity of the brush.
       * \param[in] value, value [0-100]
       *
       */
      void setBrushOpacity(int value);

      /* \brief Sets the @item used to specify the spacing of the stroke.
       * \param[in] item, view item adapter raw pointer.
       *
       */
      void setReferenceItem(ViewItemAdapterPtr item);

      /* \brief Returns the reference item used to specify the spacing of the stroke.
       *
       */
      ViewItemAdapterPtr referenceItem() const
      { return m_item; }

      /* \brief Returns the reference spacing.
       *
       */
      Spacing referenceSpacing() const;

      /* \brief Returns the mask of the pixels selected.
       *
       */
      BinaryMaskSPtr<unsigned char> voxelSelectionMask() const;

      /* \brief Aborts the current operation.
       *
       */
      void abortOperation();

    signals:
      void radiusChanged(int);
      void drawingModeChanged(bool);

    protected slots:
    	/* \brief Returns the BrushShape created with the specified parameters.
    	 * \param[in] item, view item adapter raw pointer.
    	 * \param[in] center, center of the brush.
    	 * \param[in] radius, radius of the brush.
    	 * \param[in] plane, plane of the brush.
    	 *
    	 */
      virtual BrushShape createBrushShape(ViewItemAdapterPtr item,
                                          NmVector3 center,
                                          Nm radius,
                                          Plane plane) = 0;

    	/* \brief Updates the selector when the view changes the current slice.
    	 *
    	 */
    	virtual void updateSliceChange();

    protected:
    	/* \brief Helper method to build the brush cursor.
    	 *
    	 */
      void buildCursor();

      /* \brief Returns the bounds of the brush shape in the given center.
       * \param[in] center, center of the brush.
       */
      Bounds buildBrushBounds(NmVector3 center);

      /* \brief Returns the brush center given the position in the display.
       * \param[out] center, center of the brush.
       * \param[in] pos, display position of the cursor.
       *
       */
      void getBrushPosition(NmVector3 &center, QPoint const pos);

      /* \brief Returns true if the stroke is valid given the center of the brush.
       * \param[in] center, center of the brush.
       *
       */
      bool validStroke(NmVector3 &center);

      /* \brief Updates the view when the user starts a stroke.
       * \param[in] pos, position of the brush.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void startStroke(QPoint pos, RenderView *view);

      /* \brief Updates the view when the stroke has another point.
       * \param[in] pos, position of the brush.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void updateStroke(QPoint pos, RenderView *view);

      /* \brief Updates the view when the stroke ends.
       * \param[in] pos, position of the brush.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void stopStroke(RenderView *view);

      /* \brief Starts a preview in the given view.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void startPreview(RenderView *view);

      /* \brief Updates the preview in the given view.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void updatePreview(BrushShape shape, RenderView *view);

      /* \brief Stops the preview in the given view.
       * \param[in] view, raw pointer of the view to update.
       *
       */
      virtual void stopPreview(RenderView *view);

      /* \brief Convenience method that returns true when the shift key is down in the keyboard.
       *
       */
      inline bool ShiftKeyIsDown();

  private:
      /* \brief Updates the cursor and the view when the drawing mode changes.
       * \param[in] view, raw pointer of the view.
       *
       */
      void updateCurrentDrawingMode(RenderView* view);

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

      bool           m_eraseMode;
      bool           m_drawing;
      BrushShapeList m_brushes;
      NmVector3      m_lastUdpdatePoint;
      Bounds         m_lastUpdateBounds;
      bool           m_tracking;
      RenderView*    m_previewView;

      static const int MAX_RADIUS = 30;
    };

  using BrushSelectorPtr  = BrushSelector *;
  using BrushSelectorSPtr = std::shared_ptr<BrushSelector>;

} // namespace ESPINA

#endif // ESPINA_BRUSH_SELECTOR_H
