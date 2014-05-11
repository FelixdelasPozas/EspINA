/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <GUI/View/Widgets/Contour/vtkVoxelContour2D.h>

// VTK
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTextureMapToPlane.h>
#include <vtkTransformTextureCoords.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ROIWidget::ROIWidget(ViewManagerSPtr vm)
  : m_vm     {vm}
  {
    connect(m_vm.get(), SIGNAL(ROIChanged()), this, SLOT(ROIModified()));
    Q_ASSERT(vm->currentROI() == nullptr);

    // create volume selection texture for 3D views
    vtkSmartPointer<vtkImageCanvasSource2D> textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
    textureIcon->SetScalarTypeToUnsignedChar();
    textureIcon->SetExtent(0, 15, 0, 15, 0, 0);
    textureIcon->SetNumberOfScalarComponents(4);
    textureIcon->SetDrawColor(0,0,0,0);             // transparent color
    textureIcon->FillBox(0,15,0,15);
    textureIcon->SetDrawColor(255,255,0,150);     // "somewhat transparent" yellow
    textureIcon->DrawSegment(0, 0, 15, 15);
    textureIcon->DrawSegment(1, 0, 15, 14);
    textureIcon->DrawSegment(0, 1, 14, 15);
    textureIcon->DrawSegment(15, 0, 15, 0);
    textureIcon->DrawSegment(0, 15, 0, 15);

    m_texture = vtkSmartPointer<vtkTexture>::New();
    m_texture->SetInputConnection(textureIcon->GetOutputPort());
    m_texture->RepeatOn();
    m_texture->InterpolateOn();
    m_texture->ReleaseDataFlagOn();
  }
  
  //-----------------------------------------------------------------------------
  ROIWidget::~ROIWidget()
  {
    for(auto view: m_views.keys())
      unregisterView(view);

    m_views.clear();
  }
  
  //-----------------------------------------------------------------------------
  void ROIWidget::registerView(RenderView* view)
  {
    if(m_views.keys().contains(view) || m_vm->currentROI() == nullptr)
      return;

    auto bounds = m_vm->currentROI()->bounds();

    if(isView3D(view))
    {
      vtkImageData *image = vtkImage(m_vm->currentROI()->itkImage(), bounds);

      // generate surface
      m_marchingCubes[view] = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
      m_marchingCubes[view]->SetInputData(image);
      m_marchingCubes[view]->ReleaseDataFlagOn();
      m_marchingCubes[view]->SetGlobalWarningDisplay(false);
      m_marchingCubes[view]->SetNumberOfContours(1);
      m_marchingCubes[view]->GenerateValues(1, SEG_VOXEL_VALUE, SEG_VOXEL_VALUE);
      m_marchingCubes[view]->ComputeScalarsOff();
      m_marchingCubes[view]->ComputeNormalsOff();
      m_marchingCubes[view]->ComputeGradientsOff();

      // NOTE: not using normals to render the selection because we need to represent as many voxels as possible, also
      // we don't decimate our mesh for the same reason.
      vtkSmartPointer<vtkTextureMapToPlane> textureMapper = vtkSmartPointer<vtkTextureMapToPlane>::New();
      textureMapper->SetInputConnection(m_marchingCubes[view]->GetOutputPort());
      textureMapper->SetGlobalWarningDisplay(false);
      textureMapper->AutomaticPlaneGenerationOn();

      vtkSmartPointer<vtkTransformTextureCoords> textureTrans = vtkSmartPointer<vtkTransformTextureCoords>::New();
      textureTrans->SetInputConnection(textureMapper->GetOutputPort());
      textureTrans->SetGlobalWarningDisplay(false);

      auto spacing = m_vm->currentROI()->spacing();
      auto scale = NmVector3{ (bounds[1]-bounds[0])/spacing[0], (bounds[3]-bounds[2])/spacing[1], (bounds[5]-bounds[4])/spacing[2]};
      textureTrans->SetScale(scale[0], scale[1], scale[2]);

      vtkSmartPointer<vtkPolyDataMapper> polydataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      polydataMapper->SetInputConnection(textureTrans->GetOutputPort());
      polydataMapper->SetResolveCoincidentTopologyToOff();

      vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(polydataMapper);
      actor->SetTexture(m_texture);
      actor->GetProperty()->SetOpacity(1);
      actor->SetVisibility(true);

      view->addActor(actor);
      m_views[view] = actor;
    }
    else
    {
      auto view2d = dynamic_cast<View2D *>(view);
      connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)), Qt::QueuedConnection);

      auto index = normalCoordinateIndex(view2d->plane());
      auto pos = view2d->crosshairPoint()[index];
      auto sliceBounds = bounds;
      sliceBounds[2*index] = sliceBounds[(2*index)+1] = pos;

      vtkImageData *image = vtkImage(m_vm->currentROI()->itkImage(sliceBounds), sliceBounds);
      m_contours[view] = vtkSmartPointer<vtkVoxelContour2D>::New();
      m_contours[view]->SetInputData(image);
      m_contours[view]->UpdateWholeExtent();

      vtkSmartPointer<vtkTubeFilter> tubes = vtkSmartPointer<vtkTubeFilter>::New();
      tubes->SetInputData(m_contours[view]->GetOutput());
      tubes->SetUpdateExtent(image->GetExtent());
      tubes->SetCapping(false);
      tubes->SetGenerateTCoordsToUseLength();
      tubes->SetNumberOfSides(4);
      tubes->SetOffset(1.0);
      tubes->SetOnRatio(1.5);
      tubes->UpdateWholeExtent();

      vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetInputConnection(tubes->GetOutputPort());
      mapper->SetUpdateExtent(image->GetExtent());
      mapper->SetColorModeToDefault();
      mapper->ScalarVisibilityOff();
      mapper->StaticOff();

      vtkSmartPointer<vtkImageCanvasSource2D> textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
      textureIcon->SetScalarTypeToUnsignedChar();
      textureIcon->SetExtent(0, 31, 0, 31, 0, 0);
      textureIcon->SetNumberOfScalarComponents(4);
      textureIcon->SetDrawColor(255,255,255,255);
      textureIcon->FillBox(0,31,0,31);
      textureIcon->SetDrawColor(0,0,0,0);
      textureIcon->FillBox(24, 31, 0, 7);
      textureIcon->FillBox(0, 7, 24, 31);
      textureIcon->Update();

      vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
      texture->SetInputConnection(textureIcon->GetOutputPort());
      texture->SetEdgeClamp(false);
      texture->RepeatOn();
      texture->InterpolateOff();
      texture->Modified();

      mapper->Update();

      vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);
      actor->GetProperty()->SetColor(1,1,0); // yellow
      actor->GetProperty()->SetOpacity(1);
      actor->GetProperty()->Modified();
      actor->SetVisibility(true);
      actor->SetDragable(false);
      actor->SetTexture(m_texture);
      actor->Modified();

      view->addActor(actor);
      m_views[view] = actor;
    }
  }

  //-----------------------------------------------------------------------------
  void ROIWidget::unregisterView(RenderView* view)
  {
    if(!m_views.keys().contains(view))
      return;

    view->removeActor(m_views[view]);
    m_views[view] = nullptr;
    m_views.remove(view);

    if (isView2D(view))
    {
      auto view2d = dynamic_cast<View2D *>(view);
      disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));

      m_contours[view] = nullptr;
    }
    else
    {
      m_marchingCubes[view] = nullptr;
    }
  }
  
  //-----------------------------------------------------------------------------
  void ROIWidget::setEnabled(bool enable)
  {
    for(auto actor: m_views.values())
      actor->SetVisibility(enable);
  }
  
  //-----------------------------------------------------------------------------
  void ROIWidget::sliceChanged(Plane plane, Nm pos)
  {
    auto rView = qobject_cast<RenderView *>(sender());

    for(auto view: m_views.keys())
    {
      if(view == rView)
      {
        auto view2d = dynamic_cast<View2D *>(rView);
        auto bounds = m_vm->currentROI()->bounds();
        auto index = normalCoordinateIndex(view2d->plane());
        bounds[2*index] = bounds[(2*index)+1] = view2d->crosshairPoint()[index];

        vtkImageData *image = vtkImage(m_vm->currentROI()->itkImage(bounds), bounds);

        m_contours[view]->SetInputData(image);
        m_contours[view]->Update();
        m_views[view]->GetMapper()->SetUpdateExtent(image->GetExtent());
        m_views[view]->GetMapper()->Update();
        m_views[view]->Modified();
      }
    }
  }

  //----------------------------------------------------------------------------
  void ROIWidget::ROIChanged()
  {
    if(m_vm->currentROI() == nullptr && !m_views.keys().empty())
    {
      for(auto view: m_vm->renderViews())
        unregisterView(view);

      return;
    }

    if(m_vm->currentROI() != nullptr && m_views.keys().empty())
    {
      for(auto view: m_vm->renderViews())
        registerView(view);

      return;
    }

    for(auto view: m_vm->renderViews())
    {
      vtkImageData *image = nullptr;

      if(isView3D(view))
      {
        image = vtkImage(m_vm->currentROI()->itkImage(), m_vm->currentROI()->bounds());
        m_marchingCubes[view]->SetInputData(image);
        m_marchingCubes[view]->SetUpdateExtent(image->GetExtent());
        m_marchingCubes[view]->Update();
        m_views[view]->GetMapper()->SetUpdateExtent(image->GetExtent());
      }
      else
      {
        auto view2d = dynamic_cast<View2D *>(view);
        auto bounds = m_vm->currentROI()->bounds();
        auto index = normalCoordinateIndex(view2d->plane());
        bounds[2*index] = bounds[(2*index)+1] = view2d->crosshairPoint()[index];

        image = vtkImage(m_vm->currentROI()->itkImage(bounds), bounds);
        m_contours[view]->SetInputData(image);
        m_contours[view]->Update();

      }

      m_views[view]->GetMapper()->SetUpdateExtent(image->GetExtent());
      m_views[view]->GetMapper()->Update();
      m_views[view]->Modified();
    }
  }

} /* namespace EspINA */

