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

// ESPINA
#include "StrokePainter.h"
#include <GUI/View/RenderView.h>
#include "GUI/View/View2D.h"
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>

// VTK
#include <vtkLookupTable.h>
#include <vtkImplicitFunction.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>

// C++
#include <chrono>

using namespace ESPINA;

//------------------------------------------------------------------------
StrokePainter::StrokePainter(const NmVector3 &spacing,
                             const NmVector3 &origin,
                             RenderView      *view,
                             DrawingMode      mode,
                             Brush           *brush)
: m_view       {view}
, m_origin     {origin}
, m_spacing    {spacing}
, m_strokeValue{DrawingMode::PAINTING == mode ? static_cast<unsigned char>(1) : static_cast<unsigned char>(0)}
{
  m_previewBounds = view->previewBounds(false);

  auto brushColor = brush->color();

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, brushColor.redF(), brushColor.greenF(), brushColor.blueF(), brushColor.alphaF());
  m_lut->Modified();

  int extent[6];

  auto view2d = view2D_cast(view);
  Q_ASSERT(view2d);

  for (int i = 0; i < 3; ++i)
  {
    extent[2*i]     = m_previewBounds[2*i]     / spacing[i];
    extent[2*i + 1] = m_previewBounds[2*i + 1] / spacing[i];
  }

  m_strokeCanvas = vtkSmartPointer<vtkImageData>::New();
  m_strokeCanvas->SetOrigin(origin[0], origin[1], origin[2]);
  m_strokeCanvas->SetExtent(extent);
  m_strokeCanvas->SetSpacing(spacing[0], spacing[1], spacing[2]);

  auto info = m_strokeCanvas->GetInformation();
  vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
  vtkImageData::SetNumberOfScalarComponents(1, info);
  m_strokeCanvas->SetInformation(info);
  m_strokeCanvas->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  m_strokeCanvas->Modified();

  auto imagePointer = reinterpret_cast<unsigned char *>(m_strokeCanvas->GetScalarPointer());
  memset(imagePointer, 0, m_strokeCanvas->GetNumberOfPoints());

  m_strokeCanvas->Modified();
  m_strokeCanvas->GetExtent(extent);

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputData(m_strokeCanvas);
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
  double pos[3];
  m_actor->GetPosition(pos);
  pos[normalCoordinateIndex(view2d->plane())] += 1.25 * view2d->segmentationDepth();
  m_actor->SetPosition(pos);

  connect(brush, SIGNAL(strokeUpdated(Brush::Stroke)),
          this,  SLOT(onStroke(Brush::Stroke)));
}

//------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> StrokePainter::strokeCanvas() const
{
  return m_strokeCanvas;
}

//------------------------------------------------------------------------
vtkSmartPointer<vtkProp> StrokePainter::strokeActor() const
{
  return m_actor;
}

//------------------------------------------------------------------------
void StrokePainter::onStroke(Brush::Stroke stroke)
{
  int extent[6];
  m_strokeCanvas->GetExtent(extent);

  auto isValid = [&extent](int x, int y, int z){ return (extent[0] <= x && extent[1] >= x && extent[2] <= y && extent[3] >= y && extent[4] <= z && extent[5] >= z); };

  for (auto brush: stroke)
  {
    if(!brush.second.areValid()) continue;

    auto pointBounds = intersection(m_previewBounds, brush.second);
    auto region      = equivalentRegion<itkVolumeType>(m_origin, m_spacing, pointBounds);
    auto tempImage   = define_itkImage<itkVolumeType>(m_origin, m_spacing);
    tempImage->SetRegions(region);

    itk::ImageRegionIteratorWithIndex<itkVolumeType> it(tempImage, region);
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      auto index = it.GetIndex();

      if (isValid(index[0], index[1], index[2]) && (brush.first->FunctionValue(index[0] * m_spacing[0], index[1] * m_spacing[1], index[2] * m_spacing[2]) <= 0))
      {
        auto pixel = static_cast<unsigned char*>(m_strokeCanvas->GetScalarPointer(index[0],index[1], index[2]));
        *pixel     = m_strokeValue;
      }

      ++it;
    }
  }

  m_strokeCanvas->Modified();
  m_mapToColors->Update();
  m_actor->Update();
  m_view->refresh();
}

