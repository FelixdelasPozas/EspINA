/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "SkeletonRepresentation.h"
#include "RepresentationEmptySettings.h"
#include <GUI/View/View2D.h>

// VTK
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

namespace ESPINA
{
  const Representation::Type SkeletonRepresentation::TYPE = "Skeleton";
  
  TransparencySelectionHighlighter *SkeletonRepresentation::s_highlighter = new TransparencySelectionHighlighter();

  //-----------------------------------------------------------------------------
  SkeletonRepresentation::SkeletonRepresentation(SkeletonDataSPtr skeleton, RenderView *view)
  : Representation{view}
  , m_data        {skeleton}
  , m_slice       {VTK_INT_MAX}
  {
    setType(TYPE);
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonRepresentation::setColor(const QColor& color)
  {
    Representation::setColor(color);

    if(m_actor == nullptr)
      return;

    auto segColor = s_highlighter->color(m_color, m_highlight);
    m_actor->GetProperty()->SetColor(segColor.redF(), segColor.greenF(), segColor.blueF());
    m_actor->GetProperty()->SetOpacity(segColor.alphaF());
    m_actor->Modified();
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonRepresentation::setHighlighted(bool highlighted)
  {
    Representation::setHighlighted(highlighted);

    if(m_actor == nullptr)
      return;

    auto segColor = s_highlighter->color(m_color, m_highlight);
    m_actor->GetProperty()->SetColor(segColor.redF(), segColor.greenF(), segColor.blueF());
    m_actor->GetProperty()->SetOpacity(segColor.alphaF());
    m_actor->Modified();
  }
  
  //-----------------------------------------------------------------------------
  bool SkeletonRepresentation::isInside(const NmVector3& point) const
  {
    // TODO: implementar un isInside en el data y hacer el pick y este con ese método ya que no se puede hacer un pick a los actores.
    return false;
  }
  
  //-----------------------------------------------------------------------------
  bool SkeletonRepresentation::hasActor(vtkProp* actor) const
  {
    return (m_actor.Get() == vtkActor::SafeDownCast(actor));
  }
  
  //-----------------------------------------------------------------------------
  QList<vtkProp*> SkeletonRepresentation::getActors()
  {
    QList<vtkProp*> actors;
    actors << m_actor;
    return actors;
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonRepresentation::updateVisibility(bool visible)
  {
    if(visible && needUpdate())
      updateRepresentation();

    if (m_actor != nullptr)
      m_actor->SetVisibility(visible);
  }
  
  //-----------------------------------------------------------------------------
  RepresentationSettings* SkeletonRepresentation::settingsWidget()
  {
    return new RepresentationEmptySettings();
  }

  //-----------------------------------------------------------------------------
  void SkeletonRepresentation::initializePipeline()
  {
    m_polyData = vtkSmartPointer<vtkPolyData>::New();

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputData(m_polyData);

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
  }

  //-----------------------------------------------------------------------------
  void SkeletonRepresentation::updateRepresentation()
  {
    auto view2d = dynamic_cast<View2D*>(m_view);

    if(view2d)
    {
      auto newPoints = vtkSmartPointer<vtkPoints>::New();
      auto newLines = vtkSmartPointer<vtkCellArray>::New();

      auto planeIndex = normalCoordinateIndex(view2d->plane());
      auto slice = view2d->crosshairPoint()[planeIndex];
      if(slice == m_slice)
        return;
      m_slice = slice;
      auto data = m_data->skeleton();
      auto planeSpacing = m_data->spacing()[planeIndex];
      QMap<vtkIdType, NmVector3> pointIds;
      QMap<vtkIdType, vtkIdType> newPointIds;
      auto points = data->GetPoints();
      auto lines = data->GetLines();
      double pointACoords[3]{0,0,0};
      double pointBCoords[3]{0,0,0};

      for(vtkIdType i = 0; i < data->GetLines()->GetNumberOfCells(); ++i)
      {
        vtkIdList *list = nullptr;
        lines->GetCell(i, list);

        if(list == nullptr)
          continue;

        vtkIdType pointAId = list->GetId(0);
        vtkIdType pointBId = list->GetId(1);

        points->GetPoint(pointAId, pointACoords);
        points->GetPoint(pointBId, pointBCoords);

        if((pointACoords[planeIndex] < slice && pointBCoords[planeIndex] > slice) ||
           (pointACoords[planeIndex] > slice && pointBCoords[planeIndex] < slice) ||
            areEqual(pointACoords[planeIndex], slice, planeSpacing) ||
            areEqual(pointBCoords[planeIndex], slice, planeSpacing))
        {
          if(!newPointIds.contains(pointAId))
          {
            newPointIds.insert(pointAId, newPoints->InsertNextPoint(pointACoords));
          }

          if(!newPointIds.contains(pointBId))
          {
            newPointIds.insert(pointBId, newPoints->InsertNextPoint(pointBCoords));
          }

          auto line = vtkSmartPointer<vtkLine>::New();
          line->GetPointIds()->SetId(0, newPointIds[pointAId]);
          line->GetPointIds()->SetId(1, newPointIds[pointBId]);
          newLines->InsertNextCell(line);
        }
      }

      m_polyData->SetPoints(newPoints);
      m_polyData->SetLines(newLines);
      m_mapper->Update();

      auto color = s_highlighter->color(m_color, m_highlight);
      m_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
      m_actor->GetProperty()->SetOpacity(color.alphaF());
      m_actor->GetProperty()->SetLineWidth((isHighlighted() ? 4 : 1));
      m_actor->Modified();
    }
  }

} // namespace EspINA
