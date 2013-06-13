/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

// EspINA
#include "ContourRepresentation.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>
#include <Core/EspinaTypes.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/QtWidget/VolumeView.h>
#include <GUI/vtkWidgets/vtkVoxelContour2D.h>

// VTK
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageReslice.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

namespace EspINA
{
  TransparencySelectionHighlighter *ContourRepresentation::s_highlighter = new TransparencySelectionHighlighter();

  //-----------------------------------------------------------------------------
  ContourRepresentation::ContourRepresentation(SegmentationVolumeSPtr data,
                                               EspinaRenderView      *view)
  : SegmentationGraphicalRepresentation(view)
  , m_data(data)
  {
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::setColor(const QColor &color)
  {
    GraphicalRepresentation::setColor(color);

    if (m_actor != NULL)
    {
      vtkSmartPointer<vtkLookupTable> lut = s_highlighter->lut(m_color, m_highlight);

      double rgba[4];
      s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

      m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
      m_actor->GetProperty()->SetOpacity(rgba[3]);
      m_actor->GetProperty()->Modified();
    }
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::setHighlighted(bool highlighted)
  {
    SegmentationGraphicalRepresentation::setHighlighted(highlighted);

    if (m_actor != NULL)
    {
      double rgba[4];
      s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

      m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
      m_actor->GetProperty()->SetOpacity(rgba[3]);
      m_actor->GetProperty()->Modified();
    }
  }

  //-----------------------------------------------------------------------------
  bool ContourRepresentation::hasActor(vtkProp *actor) const
  {
    if (m_actor == NULL)
      return false;

    return m_actor.GetPointer() == actor;
  }

  //-----------------------------------------------------------------------------
  bool ContourRepresentation::isInside(Nm point[3])
  {
    Q_ASSERT(m_data.get());
    Q_ASSERT(m_data->toITK().IsNotNull());

    itkVolumeType::IndexType voxel = m_data->index(point[0], point[1], point[2]);

    return m_data->volumeRegion().IsInside(voxel)
        && m_data->toITK()->GetPixel(voxel) == SEG_VOXEL_VALUE;
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::initializePipeline()
  {
    connect(m_data.get(), SIGNAL(representationChanged()),
            this, SLOT(updatePipelineConnections()));

    SliceView *view = reinterpret_cast<SliceView *>(m_view);

    m_reslice = vtkSmartPointer<vtkImageReslice>::New();
    m_reslice->SetInputConnection(m_data->toVTK());
    m_reslice->SetOutputDimensionality(2);
    m_reslice->SetResliceAxes(slicingMatrix(view));
    m_reslice->AutoCropOutputOn();
    m_reslice->SetNumberOfThreads(1);
    m_reslice->Update();

    m_voxelContour = vtkSmartPointer<vtkVoxelContour2D>::New();
    m_voxelContour->SetInputConnection(m_reslice->GetOutputPort());
    m_voxelContour->Update();

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputConnection(m_voxelContour->GetOutputPort());
    m_mapper->SetColorModeToDefault();
    m_mapper->ScalarVisibilityOff();
    m_mapper->StaticOff();
    m_mapper->Update();

    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    m_actor->GetProperty()->SetLineWidth(5);
    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();
    m_actor->Modified();

    // need to reposition the actor so it will always be over the channels actors'
    double pos[3];
    m_actor->GetPosition(pos);
    pos[view->plane()] = view->segmentationDepth();
    m_actor->SetPosition(pos);
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updateRepresentation()
  {
    if (m_actor != NULL)
    {
      m_reslice->Update();
      m_mapper->Update();
      m_actor->Modified();
    }
  }

  //-----------------------------------------------------------------------------
  QList<vtkProp*> ContourRepresentation::getActors()
  {
    QList<vtkProp *> list;

    if (m_actor == NULL)
      initializePipeline();

    list << m_actor.GetPointer();

    return list;
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSPtr ContourRepresentation::cloneImplementation(SliceView *view)
  {
    ContourRepresentation *representation = new ContourRepresentation(m_data, view);
    representation->setView(view);

    return GraphicalRepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updateVisibility(bool visible)
  {
    if (m_actor != NULL)
      m_actor->SetVisibility(visible);
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updatePipelineConnections()
  {
    if (m_actor == NULL)
      return;

    if (m_reslice->GetInputConnection(0,0) != m_data->toVTK())
    {
      m_reslice->SetInputConnection(m_data->toVTK());
      m_reslice->Update();
    }
  }

} /* namespace EspINA */
