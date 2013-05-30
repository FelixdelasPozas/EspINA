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

// VTK
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkContourFilter.h>
#include <vtkImageReslice.h>
#include <vtkProperty.h>
#include <vtkCleanPolyData.h>
#include <vtkImageData.h>
#include <vtkPolyDataSilhouette.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkImageContinuousDilate3D.h>

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

    vtkSmartPointer<vtkLookupTable> lut = s_highlighter->lut(m_color, m_highlight);

    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::setHighlighted(bool highlighted)
  {
    SegmentationGraphicalRepresentation::setHighlighted(highlighted);

    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();
  }

  //-----------------------------------------------------------------------------
  bool ContourRepresentation::hasActor(vtkProp *actor) const
  {
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
  void ContourRepresentation::initializePipeline(SliceView *view)
  {
    connect(m_data.get(), SIGNAL(representationChanged()),
            this, SLOT(updatePipelineConnections()));

    vtkImageData *vtkImage = vtkImageData::SafeDownCast(m_data->toVTK()->GetProducer()->GetOutputDataObject(0));
    vtkImage->GetExtent(m_extent);

    m_reslice = vtkSmartPointer<vtkImageReslice>::New();
    m_reslice->SetInputConnection(m_data->toVTK());
    m_reslice->SetOutputDimensionality(2);
    m_reslice->InterpolateOff();
    m_reslice->AutoCropOutputOn();
    m_reslice->SetResliceAxes(slicingMatrix(view));
    m_reslice->SetNumberOfThreads(1);
    m_reslice->Update();

    // we need to dilate the border of the slice image by one (later in the dilate)
    // and also have a background border of one voxel, that's the reason for the add/subtract 2
    int extent[6];
    vtkImageData *image = m_reslice->GetOutput();
    image->GetExtent(extent);
    extent[0] -= 2;
    extent[1] += 2;
    extent[2] -= 2;
    extent[3] += 2;
    extent[4] -= 2;
    extent[5] += 2;

    m_pad = vtkSmartPointer<vtkImageConstantPad>::New();
    m_pad->SetInputConnection(m_reslice->GetOutputPort());
    m_pad->SetOutputWholeExtent(extent);
    m_pad->SetConstant(SEG_BG_VALUE);
    m_pad->Update();

    vtkSmartPointer<vtkImageContinuousDilate3D> dilate = vtkSmartPointer<vtkImageContinuousDilate3D>::New();
    dilate->SetInputConnection(m_pad->GetOutputPort());
    dilate->SetKernelSize(3,3,1);
    dilate->Update();

    vtkSmartPointer<vtkContourFilter> contour = vtkSmartPointer<vtkContourFilter>::New();
    contour->SetInputConnection(dilate->GetOutputPort());
    contour->ComputeGradientsOff();
    contour->ComputeNormalsOff();
    contour->ComputeScalarsOff();
    contour->SetNumberOfContours(1);
    contour->UseScalarTreeOn(); // UseScalarTreeOn() uses more memory, but extracts the contour faster
    contour->SetValue(0, SEG_VOXEL_VALUE);
    contour->Update();

    vtkSmartPointer<vtkPolyDataSilhouette> silhouette = vtkSmartPointer<vtkPolyDataSilhouette>::New();
    silhouette->SetCamera(view->renderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera());
    silhouette->SetInputConnection(contour->GetOutputPort());
    silhouette->BorderEdgesOn();
    silhouette->Update();

    m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_mapper->SetInputConnection(silhouette->GetOutputPort());
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

    m_view = view;
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updateRepresentation()
  {
    m_reslice->Update();
    m_pad->Update();
    m_mapper->Update();
    m_actor->Modified();
  }

  //-----------------------------------------------------------------------------
  QList<vtkProp*> ContourRepresentation::getActors()
  {
    QList<vtkProp *> list;
    list << m_actor.GetPointer();

    return list;
  }

  //-----------------------------------------------------------------------------
  GraphicalRepresentationSPtr ContourRepresentation::cloneImplementation(SliceView *view)
  {
    ContourRepresentation *representation = new ContourRepresentation(m_data, view);

    representation->initializePipeline(view);

    return GraphicalRepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updateVisibility(bool visible)
  {
    if (m_actor) m_actor->SetVisibility(visible);
  }

  //-----------------------------------------------------------------------------
  void ContourRepresentation::updatePipelineConnections()
  {
    if (m_reslice->GetInputConnection(0,0) != m_data->toVTK())
    {
      m_reslice->SetInputConnection(m_data->toVTK());
      m_reslice->Update();
    }

    int extent[6];
    vtkImageData *image = vtkImageData::SafeDownCast(m_data->toVTK()->GetProducer()->GetOutputDataObject(0));
    image->GetExtent(extent);

    if (memcmp(m_extent, extent, 6*sizeof(int)) != 0)
    {
      m_reslice->UpdateWholeExtent();
      m_reslice->Update();
      image = m_reslice->GetOutput();
      image->GetExtent(extent);
      extent[0] -= 2;
      extent[1] += 2;
      extent[2] -= 2;
      extent[3] += 2;
      extent[4] -= 2;
      extent[5] += 2;

      m_pad->SetOutputWholeExtent(extent);
      m_pad->UpdateWholeExtent();
      m_pad->Update();
    }


  }

} /* namespace EspINA */
