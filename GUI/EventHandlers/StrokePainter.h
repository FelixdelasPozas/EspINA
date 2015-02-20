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

  class StrokePainter
  : public QObject
  {
    Q_OBJECT

  public:
    StrokePainter(const NmVector3 &spacing,
                  const NmVector3 &origin,
                  RenderView      *view,
                  Brush           *brush);

    vtkSmartPointer<vtkProp> strokeActor() const;

  private slots:
    void onStroke(Brush::Stroke stroke);

  private:
    RenderView *m_view;
    NmVector3   m_origin;
    NmVector3   m_spacing;
    Bounds      m_previewBounds;
    vtkSmartPointer<vtkImageData>        m_preview;
    vtkSmartPointer<vtkLookupTable>      m_lut;
    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;
  };

  using StrokePainterSPtr = std::shared_ptr<StrokePainter>;
}

#endif // ESPINA_STROKE_PAINTER_H
