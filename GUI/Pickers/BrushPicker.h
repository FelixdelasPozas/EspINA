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


#ifndef BRUSHPICKER_H
#define BRUSHPICKER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/Pickers/ISelector.h"

// VTK
#include <vtkSmartPointer.h>

// Qt
#include <QColor>

class vtkLookupTable;
class vtkImageResliceToColors;
class vtkImageActor;
class ImplicitImageSource;
class vtkImageData;
class Channel;
class PickableItem;

namespace EspINA
{
  class EspinaGUI_EXPORT BrushPicker: public ISelector
  {
	Q_OBJECT
	typedef itkVolumeType::SpacingType Spacing;

	public:
	  explicit BrushPicker(PickableItemPtr item = NULL);
	  virtual ~BrushPicker();

	  virtual bool filterEvent(QEvent* e, EspinaRenderView* view = NULL);

	  /// @radius of the brush in screen pixels
	  void setRadius(int radius);
	  int radius() const
	  {
		return m_displayRadius;
	  }
	  void setBorderColor(QColor color);
	  void setBrushColor(QColor color);
	  void setBrushImage(QImage &image);
	  QColor getBrushColor();

	  /// @item is used to specify the spacing of the stroke
	  void setReferenceItem(PickableItemPtr item);
	  itkVolumeType::SpacingType referenceSpacing() const;

	  void DrawingOn(EspinaRenderView *view);
	  void DrawingOff(EspinaRenderView *view,
					  SegmentationPtr segmentation);

	signals:
	  void stroke(PickableItemPtr, double, double, double, Nm, PlaneType);
	  void stroke(PickableItemPtr, ISelector::WorldRegion, Nm, PlaneType);

	private:
	  void buildCursor();
	  void createBrush(double brush[3], QPoint pos);
	  bool validStroke(double brush[3]);
	  void startStroke(QPoint pos, EspinaRenderView *view);
	  void updateStroke(QPoint pos, EspinaRenderView *view);
	  void stopStroke(EspinaRenderView *view);
	  void startPreview(EspinaRenderView *view);
	  void updatePreview(double brush[3], EspinaRenderView *view);
	  void stopPreview(EspinaRenderView *view);

	private:
	  PickableItemPtr m_referenceItem;

	  int m_displayRadius; //In screen pixels
	  QColor m_borderColor;
	  QColor m_brushColor;
	  QImage *m_brushImage;

	  bool m_tracking;
	  QPoint m_lastDot;
	  ISelector::WorldRegion m_stroke;

	  PlaneType m_plane;
	  Nm m_radius;
	  Spacing m_spacing;
	  int m_viewSize[2];
	  double m_LL[3], m_UR[3];
	  Nm m_pBounds[6];
	  double m_worldSize[2];

	  vtkSmartPointer<vtkLookupTable> m_lut;
	  vtkSmartPointer<vtkImageData> m_preview;
	  vtkSmartPointer<vtkImageActor> m_actor;
	  bool m_drawing;
	  SegmentationPtr m_segmentation;

	  static const int MAX_RADIUS = 32;
	};

} // namespace EspINA

#endif // BRUSHPICKER_H
