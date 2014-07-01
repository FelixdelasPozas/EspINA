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

// EspINA
#include <GUI/View/View2D.h>
#include "SphericalBrushROISelector.h"

// VTK
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>

// Qt
#include <QEvent>
#include <QMouseEvent>

namespace EspINA
{
  //----------------------------------------------------------------------------
  SphericalBrushROISelector::SphericalBrushROISelector()
  : m_hasROI{false}
  {
  }

  //----------------------------------------------------------------------------
  SphericalBrushROISelector::~SphericalBrushROISelector()
  {
  }
  
  //----------------------------------------------------------------------------
  bool SphericalBrushROISelector::filterEvent(QEvent* e, RenderView* view)
  {
    QKeyEvent *ke = nullptr;

    switch(e->type())
    {
      case QEvent::KeyPress:
        {
          ke = static_cast<QKeyEvent *>(e);
          if ((ke->key() == Qt::Key_Shift) && !m_tracking && m_hasROI)
          {
            m_drawing = false;
            buildCursor();
            view->setCursor(m_cursor);
            startPreview(view);

            emit drawingModeChanged(m_drawing);
            return true;
          }
        }
        break;
      default:
        break;
    }

    return BrushSelector::filterEvent(e, view);;
  }

  //----------------------------------------------------------------------------
  void SphericalBrushROISelector::startPreview(RenderView *view)
  {
    if (m_previewView != nullptr)
      return;

    NmVector3 spacing{m_spacing[0], m_spacing[1], m_spacing[2]};
    VolumeBounds previewBounds{ view->previewBounds(false), spacing, m_origin};
    m_previewView = view;

    m_lut = vtkSmartPointer<vtkLookupTable>::New();
    m_lut->Allocate();
    m_lut->SetNumberOfTableValues(2);
    m_lut->Build();
    m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
    m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), m_brushOpacity/100.);
    m_lut->Modified();

    int extent[6];
    for (int i = 0; i < 3; ++i)
    {
      extent[2 * i]       = m_pBounds[2 * i]       / m_spacing[i];
      extent[(2 * i) + 1] = m_pBounds[(2 * i) + 1] / m_spacing[i];
    }
    m_preview = vtkSmartPointer<vtkImageData>::New();
    m_preview->SetOrigin(0, 0, 0);
    vtkInformation *info = m_preview->GetInformation();
    m_preview->SetExtent(extent);
    m_preview->SetSpacing(m_spacing[0], m_spacing[1], m_spacing[2]);
    vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
    vtkImageData::SetNumberOfScalarComponents(1, info);
    m_preview->SetInformation(info);
    m_preview->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    m_preview->Modified();
    unsigned char *imagePointer = reinterpret_cast<unsigned char *>(m_preview->GetScalarPointer());
    memset(imagePointer, 0, m_preview->GetNumberOfPoints());
    m_previewBounds = previewBounds.bounds();
    m_preview->Modified();
    m_preview->GetExtent(extent);

    m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
    m_mapToColors->SetInputData(m_preview);
    m_mapToColors->SetUpdateExtent(extent);
    m_mapToColors->SetLookupTable(m_lut);
    m_mapToColors->SetNumberOfThreads(1);
    m_mapToColors->Update();

    m_actor = vtkSmartPointer<vtkImageActor>::New();
    m_actor->SetPickable(false);
    m_actor->SetDisplayExtent(extent);
    m_actor->SetInterpolate(false);
    m_actor->GetMapper()->SetNumberOfThreads(1);
    m_actor->GetMapper()->BorderOn();
    m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
    m_actor->GetMapper()->SetUpdateExtent(extent);
    m_actor->Update();

    // preview actor must be above others or it will be occluded
    View2D* view2d = qobject_cast<View2D *>(m_previewView);
    double pos[3];
    m_actor->GetPosition(pos);
    int index = normalCoordinateIndex(view2d->plane());
    pos[index] = pos[index] + ((index == 2) ? -View2D::SEGMENTATION_SHIFT : View2D::SEGMENTATION_SHIFT);
    m_actor->SetPosition(pos);

    m_previewView->addActor(m_actor);
    m_previewView->updateView();
  }

  //----------------------------------------------------------------------------
  void SphericalBrushROISelector::updatePreview(NmVector3 center, RenderView *view)
  {
    if (m_previewView == nullptr)
      startPreview(view);

    Bounds brushBounds = buildBrushBounds(center);
    QList<NmVector3> points;
    double r2 = m_radius*m_radius;

    if (intersect(brushBounds, m_previewBounds))
    {
      double point1[3] = { static_cast<double>(m_lastUdpdatePoint[0]), static_cast<double>(m_lastUdpdatePoint[1]), static_cast<double>(m_lastUdpdatePoint[2])};
      double point2[3] = { static_cast<double>(center[0]),static_cast<double>(center[1]),static_cast<double>(center[2]) };
      double distance = vtkMath::Distance2BetweenPoints(point1,point2);

      // apply stroke interpolation
      if ((distance >= r2) && m_lastUpdateBounds.areValid())
      {
        m_brushes.pop_back(); // we must delete the last one because we are going to replace it;

        double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
        int chunks = 2* static_cast<int>(distance/r2);
        double delta[3] = { vector[0]/chunks, vector[1]/chunks, vector[2]/chunks };
        for(auto i = 0; i < chunks; ++i)
        {
          points << NmVector3{m_lastUdpdatePoint[0] + static_cast<int>(delta[0] * i),
                              m_lastUdpdatePoint[1] + static_cast<int>(delta[1] * i),
                              m_lastUdpdatePoint[2] + static_cast<int>(delta[2] * i)};

          m_brushes << createBrushShape(m_item, points.last(), m_radius, m_plane);
        }
      }
      else
        points << center;

      int extent[6];
      m_preview->GetExtent(extent);
      for (auto point: points)
      {
        Bounds brushBounds = buildBrushBounds(point);
        if (!intersect(m_previewBounds, brushBounds))
          continue;

        Bounds pointBounds = intersection(m_previewBounds,brushBounds);
        double pointCenter[3]{ point[0], point[1], point[2] };
        int depth;


        switch(m_plane)
        {
          case Plane::XY:
            depth = vtkMath::Round(((m_previewBounds[4]+m_previewBounds[5])/2)/m_spacing[2]);
            for (int x = vtkMath::Round((pointBounds[0]+m_spacing[0]/2)/m_spacing[0]); x < vtkMath::Round((pointBounds[1]+m_spacing[0]/2)/m_spacing[0]); x++)
              for (int y = vtkMath::Round((pointBounds[2]+m_spacing[1]/2)/m_spacing[1]); y < vtkMath::Round((pointBounds[3]+m_spacing[1]/2)/m_spacing[1]); y++)
            {
              if (x < extent[0] || x > extent[1])
                continue;
              if (y < extent[2] || y > extent[3])
                continue;

              double pixel[3]{x*m_spacing[0], y*m_spacing[1], m_previewBounds[4]};
              if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
              {
                unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,y,depth));
                *pixel = 1;
              }
            }
            break;
          case Plane::XZ:
            depth = vtkMath::Round(((m_previewBounds[2]+m_previewBounds[3])/2)/m_spacing[1]);
            for (int x = vtkMath::Round((pointBounds[0]+m_spacing[0]/2)/m_spacing[0]); x < vtkMath::Round((pointBounds[1]+m_spacing[0]/2)/m_spacing[0]); x++)
              for (int z = vtkMath::Round((pointBounds[4]+m_spacing[2]/2)/m_spacing[2]); z < vtkMath::Round((pointBounds[5]+m_spacing[2]/2)/m_spacing[2]); z++)
            {
              if (x < extent[0] || x > extent[1])
                continue;
              if (z < extent[4] || z > extent[5])
                continue;

              double pixel[3] = {x*m_spacing[0], m_previewBounds[2], z*m_spacing[2]};
              if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
              {
                unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,depth,z));
                *pixel = 1;
              }
            }
            break;
          case Plane::YZ:
            depth = vtkMath::Round(((m_previewBounds[0]+m_previewBounds[1])/2)/m_spacing[0]);
            for (int y = vtkMath::Round((pointBounds[2]+m_spacing[1]/2)/m_spacing[1]); y < vtkMath::Round((pointBounds[3]+m_spacing[1]/2)/m_spacing[1]); y++)
              for (int z = vtkMath::Round((pointBounds[4]+m_spacing[2]/2)/m_spacing[2]); z < vtkMath::Round((pointBounds[5]+m_spacing[2]/2)/m_spacing[2]); z++)
            {
              if (y < extent[2] || y > extent[3])
                continue;
              if (z < extent[4] || z > extent[5])
                continue;

              double pixel[3] = {m_previewBounds[0], y*m_spacing[1], z*m_spacing[2]};
              if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
              {
                unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(depth,y,z));
                *pixel = 1;
              }
            }
            break;
          default:
            break;
        }
      }
      m_lastUpdateBounds = brushBounds;
      m_preview->Modified();
      m_mapToColors->Update();
      m_actor->Update();
      m_previewView->updateView();
    }

  }

  //-----------------------------------------------------------------------------
  void SphericalBrushROISelector::stopStroke(RenderView* view)
  {
    if(!m_item)
      return;

    if (!m_brushes.empty())
    {
      auto mask = voxelSelectionMask();
      Selector::SelectionItem item{QPair<SelectionMask, NeuroItemAdapterPtr>{mask, m_item}};
      Selector::Selection selection;
      selection << item;

      emit itemsSelected(selection);
    }

    stopPreview(view);
    buildCursor();
    view->setCursor(m_cursor);
    emit drawingModeChanged(m_drawing);
    view->updateView();

    m_brushes.clear();
  }

} // namespace EspINA
