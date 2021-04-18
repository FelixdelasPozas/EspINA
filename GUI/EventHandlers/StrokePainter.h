/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_STROKE_PAINTER_H
#define ESPINA_STROKE_PAINTER_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <vtkSmartPointer.h>
#include <vtkProp.h>
#include <GUI/Model/ViewItemAdapter.h>
#include "Brush.h"
#include <QColor>

class vtkImageActor;
class vtkImageData;
class vtkImageMapToColors;
class vtkLookupTable;

namespace ESPINA
{
  class RenderView;

  enum class DrawingMode : int8_t
  {
    PAINTING,
    ERASING
  };

  class EspinaGUI_EXPORT StrokePainter
  : public QObject
  {
    Q_OBJECT

  public:
    /** \brief StrokePainter class constructor.
     * \param[in] spacing spacing of the stroke canvas image.
     * \param[in] origin origin of  the stroke canvas image.
     * \param[in] view view where the stroke actor will be shown.
     * \param[in] mode mode of the stroke.
     * \param[in] brush brush object that will make the stroke.
     *
     */
    StrokePainter(const NmVector3 &spacing,
                  const NmVector3 &origin,
                  RenderView      *view,
                  DrawingMode      mode,
                  Brush           *brush);

    /** \brief Returns the canvas image.
     *
     */
    vtkSmartPointer<vtkImageData> strokeCanvas() const;

    /** \brief Returns the stroke actor.
     *
     */
    vtkSmartPointer<vtkProp> strokeActor() const;

    /** \brief Overrides the default value to paint the stroke.
     * \param[in] value unsigned char value to use as stroke value.
     *
     */
    void overrideStrokeValue(unsigned char value)
    { m_strokeValue = value; }

  private slots:
    void onStroke(Brush::Stroke stroke);

  private:
    RenderView *m_view;
    NmVector3   m_origin;
    NmVector3   m_spacing;
    Bounds      m_previewBounds;


    unsigned char m_strokeValue;
    vtkSmartPointer<vtkImageData>        m_strokeCanvas;
    vtkSmartPointer<vtkLookupTable>      m_lut;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;
  };

  using StrokePainterSPtr = std::shared_ptr<StrokePainter>;
}

#endif // ESPINA_STROKE_PAINTER_H
