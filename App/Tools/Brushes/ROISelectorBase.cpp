/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/View/View2D.h>
#include <Tools/Brushes/ROISelectorBase.h>

// VTK
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImplicitFunction.h>

// Qt
#include <QEvent>
#include <QMouseEvent>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  ROISelectorBase::ROISelectorBase()
  : m_hasROI{false}
  {
  }

  //----------------------------------------------------------------------------
  ROISelectorBase::~ROISelectorBase()
  {
  }

  //----------------------------------------------------------------------------
  bool ROISelectorBase::filterEvent(QEvent* e, RenderView* view)
  {
    switch(e->type())
    {
      case QEvent::KeyPress:
        {
          auto ke = static_cast<QKeyEvent *>(e);

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

    return BrushSelector::filterEvent(e, view);
  }

  //----------------------------------------------------------------------------
  void ROISelectorBase::startPreview(RenderView *view)
  {
    if (m_previewView != nullptr)
      return;

    m_pBounds = view->previewBounds(false);
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
    pos[index] += 2* view2d->segmentationDepth();
    m_actor->SetPosition(pos);

    m_previewView->addActor(m_actor);
    m_previewView->updateView();
  }

  //----------------------------------------------------------------------------
  void ROISelectorBase::updatePreview(BrushShape shape, RenderView *view)
  {
    if (m_previewView == nullptr)
      startPreview(view);

    Bounds brushBounds = shape.second;
    NmVector3 center{(brushBounds[0]+brushBounds[1])/2, (brushBounds[2]+brushBounds[3])/2, (brushBounds[4]+brushBounds[5])/2};
    double r2 = m_radius*m_radius;

    if (intersect(brushBounds, m_pBounds))
    {
      double point1[3] = { static_cast<double>(m_lastUdpdatePoint[0]), static_cast<double>(m_lastUdpdatePoint[1]), static_cast<double>(m_lastUdpdatePoint[2])};
      double point2[3] = { center[0], center[1], center[2] };
      double distance = vtkMath::Distance2BetweenPoints(point1,point2);

      BrushShapeList brushes;
      brushes << shape;

      // apply stroke interpolation
      if ((distance >= r2) && m_lastUpdateBounds.areValid())
      {
        brushes.clear(); // we are going to replace it with a list of brushes.

        double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
        int chunks = 2* static_cast<int>(distance/r2);
        double delta[3] = { vector[0]/chunks, vector[1]/chunks, vector[2]/chunks };
        for(auto i = 0; i < chunks; ++i)
        {
          auto pointCenter = NmVector3{m_lastUdpdatePoint[0] + static_cast<int>(delta[0] * i),
                                       m_lastUdpdatePoint[1] + static_cast<int>(delta[1] * i),
                                       m_lastUdpdatePoint[2] + static_cast<int>(delta[2] * i)};

          brushes << createBrushShape(m_item, pointCenter, m_radius, m_plane);
        }
      }

      int extent[6];
      m_preview->GetExtent(extent);
      NmVector3 nmSpacing{m_spacing[0], m_spacing[1], m_spacing[2]};
      for (auto brush: brushes)
      {
        if (!intersect(m_pBounds, brush.second))
        {
          brushes.removeOne(brush);
          continue;
        }

        Bounds pointBounds = intersection(m_pBounds, brush.second);
        auto region = equivalentRegion<itkVolumeType>(m_origin, nmSpacing, pointBounds);
        auto tempImage = create_itkImage<itkVolumeType>(pointBounds, SEG_BG_VALUE, nmSpacing, m_origin);

        itk::ImageRegionIteratorWithIndex<itkVolumeType> it(tempImage, region);
        it.GoToBegin();
        while(!it.IsAtEnd())
        {
          auto index = it.GetIndex();

          if (!(index[0] < extent[0] || index[0] > extent[1] || index[1] < extent[2] || index[1] > extent[3] || index[2] < extent[4] || index[2] > extent[5])
             && (brush.first->FunctionValue(index[0] * m_spacing[0], index[1] * m_spacing[1], index[2] * m_spacing[2]) <= 0))
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(index[0],index[1], index[2]));
            *pixel = 1;
          }

          ++it;
        }
      }

      m_brushes << brushes;
      m_lastUpdateBounds = brushBounds;
      m_preview->Modified();
      m_mapToColors->Update();
      m_actor->Update();
      m_previewView->updateView();
    }
  }

  //-----------------------------------------------------------------------------
  void ROISelectorBase::stopStroke(RenderView* view)
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

} // namespace ESPINA
