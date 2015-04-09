/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <Core/Utils/Spatial.h>
#include <GUI/Representations/Managers/CrosshairManager.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLine.h>
#include <vtkProperty.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  CrosshairManager::CrosshairManager()
  : RepresentationManager{ViewType::VIEW_2D|ViewType::VIEW_3D}
  , m_init               {false}
  , m_actorsInUse        {false}
  {
    double colors[3][3]{ { 0, 1, 1 }, { 0, 0, 1 },  { 1, 0, 1 } };

    for(auto index: {0,1,2})
    {
      m_points[index]  = vtkSmartPointer<vtkPoints>::New();
      m_cells[index]   = vtkSmartPointer<vtkCellArray>::New();

      m_datas[index]   = vtkSmartPointer<vtkPolyData>::New();
      m_datas[index]->SetPoints(m_points[index]);
      m_datas[index]->SetLines(m_cells[index]);

      m_mappers[index] = vtkSmartPointer<vtkPolyDataMapper>::New();
      m_mappers[index]->SetInputData(m_datas[index]);

      m_actors[index]  = vtkSmartPointer<vtkActor>::New();
      m_actors[index]->SetMapper(m_mappers[index]);
      m_actors[index]->GetProperty()->SetPointSize(2);
      m_actors[index]->GetProperty()->SetLineWidth(1);
      m_actors[index]->GetProperty()->SetColor(colors[index]);
      m_actors[index]->SetPickable(false);
    }

    setName(QObject::tr("Crosshair representation"));
    setDescription(QObject::tr("Shows/Hides the crosshair in the views."));
    setIcon(QIcon(":espina/crosshair_planes.svg"));
  }
  
  //-----------------------------------------------------------------------------
  TimeRange CrosshairManager::readyRange() const
  {
    Q_ASSERT(false);
    return TimeRange();
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterPtr CrosshairManager::pick(const NmVector3 &point, vtkProp *actor) const
  {
    return nullptr;
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::setCrosshair(const NmVector3 &crosshair, TimeStamp time)
  {
    if(crosshair == m_crosshair && m_init) return;

    m_init = true;

    if(m_view)
    {
      if(view3D_cast(m_view))
      {
        configure3DActors(crosshair);
      }
      else
      {
        configure2DActors(crosshair);
      }

      emitRenderRequest(time);
    }
  }
  //-----------------------------------------------------------------------------
  void CrosshairManager::onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t)
  {
    ESPINA::RepresentationManager::onSceneResolutionChanged(resolution, t);
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::onSceneBoundsChanged(const Bounds &bounds, TimeStamp t)
  {
    ESPINA::RepresentationManager::onSceneBoundsChanged(bounds, t);
  }


  //-----------------------------------------------------------------------------
  void CrosshairManager::onShow(TimeStamp t)
  {
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::onHide(TimeStamp t)
  {
  }


  //-----------------------------------------------------------------------------
  RepresentationManagerSPtr CrosshairManager::cloneImplementation()
  {
    auto manager = std::make_shared<CrosshairManager>();
    manager->setName(this->name());
    manager->setDescription(this->description());
    manager->setIcon(this->icon());

    return manager;
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::configure2DActors(const NmVector3 &crosshair)
  {
    Nm hShift, vShift, zShift;
    auto res         = m_view->sceneResolution();
    auto sceneBounds = m_view->sceneBounds();
    auto view        = view2D_cast(m_view);
    auto depth       = view->widgetDepth();

    switch(view->plane())
    {
      case Plane::XY:
      {
        hShift = 0.5*res[0];
        vShift = 0.5*res[1];
        zShift = crosshair[2]+depth;

        double p1[3]{sceneBounds[0]-hShift, crosshair[1], zShift};
        double p2[3]{sceneBounds[1]+hShift, crosshair[1], zShift};
        setPointInternal(0, p1, p2);

        double p3[3]{crosshair[0], sceneBounds[2]-vShift, zShift};
        double p4[3]{crosshair[0], sceneBounds[3]+vShift, zShift};
        setPointInternal(1, p3, p4);
      }
        break;
      case Plane::XZ:
      {
        hShift = 0.5*res[0];
        vShift = 0.5*res[2];
        zShift = crosshair[1]+depth;

        double p1[3]{sceneBounds[0]-hShift, zShift, crosshair[2]};
        double p2[3]{sceneBounds[1]+hShift, zShift, crosshair[2]};
        setPointInternal(0, p1, p2);

        double p3[3]{crosshair[0], zShift, sceneBounds[4]-vShift};
        double p4[3]{crosshair[0], zShift, sceneBounds[5]+vShift};
        setPointInternal(2, p3, p4);
      }
        break;
      case Plane::YZ:
      {
        hShift = 0.5*res[2];
        vShift = 0.5*res[1];
        zShift = crosshair[0]+depth;

        double p1[3]{zShift, crosshair[1], sceneBounds[4]-hShift};
        double p2[3]{zShift, crosshair[1], sceneBounds[5]+hShift};
        setPointInternal(1, p1, p2);

        double p3[3]{zShift, sceneBounds[2]-vShift, crosshair[2]};
        double p4[3]{zShift, sceneBounds[3]+vShift, crosshair[2]};
        setPointInternal(2, p3, p4);
      }
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::configure3DActors(const NmVector3 &crosshair)
  {
    auto sceneBounds = m_view->sceneBounds();

    for(auto index: {0,1,2})
    {
      auto reslicePoint = crosshair[index];

      if (sceneBounds[2*index] <= reslicePoint && reslicePoint < sceneBounds[2*index+1])
      {
        auto sliceBounds = sceneBounds;
        sliceBounds.setLowerInclusion(true);
        sliceBounds.setUpperInclusion(toAxis(index), true);
        sliceBounds[2*index] = sliceBounds[2*index+1] = reslicePoint;

        double cp0[3] = { sliceBounds[0], sliceBounds[2], sliceBounds[4] };
        double cp1[3] = { sliceBounds[1], sliceBounds[2], sliceBounds[4] };
        double cp2[3] = { sliceBounds[1], sliceBounds[3], sliceBounds[4] };
        double cp3[3] = { sliceBounds[1], sliceBounds[3], sliceBounds[5] };
        double cp4[3] = { sliceBounds[0], sliceBounds[3], sliceBounds[5] };
        double cp5[3] = { sliceBounds[0], sliceBounds[2], sliceBounds[5] };

        m_points[index]->Reset();
        m_points[index]->InsertNextPoint(cp0);
        m_points[index]->InsertNextPoint(cp1);
        m_points[index]->InsertNextPoint(cp2);
        m_points[index]->InsertNextPoint(cp3);
        m_points[index]->InsertNextPoint(cp4);
        m_points[index]->InsertNextPoint(cp5);
        m_points[index]->InsertNextPoint(cp0);
        m_points[index]->Modified();

        m_cells[index]->Reset();
        for (unsigned int i = 0; i < 6; i++)
        {
          auto line = vtkLine::New();
          line->GetPointIds()->SetId(0, i);
          line->GetPointIds()->SetId(1, i + 1);
          m_cells[index]->InsertNextCell(line);
          line->Delete();
        }
        m_cells[index]->Modified();
        m_datas[index]->Modified();
        m_mappers[index]->Update();
        m_actors[index]->Modified();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::setPointInternal(int index, double *point1, double *point2)
  {
    m_points[index]->Reset();
    m_points[index]->InsertNextPoint(point1);
    m_points[index]->InsertNextPoint(point2);
    m_points[index]->Modified();
    m_cells[index]->Reset();
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, 0);
    line->GetPointIds()->SetId(1, 1);
    m_cells[index]->InsertNextCell(line);
    m_cells[index]->Modified();
    m_datas[index]->Modified();
    m_mappers[index]->Update();
    m_actors[index]->Modified();
  }

  //-----------------------------------------------------------------------------
  void CrosshairManager::displayImplementation(TimeStamp time)
  {
    bool needsRender = (representationsShown() && !m_actorsInUse) || (!representationsShown() && m_actorsInUse);

    if (representationsShown())
    {
      if(m_view->crosshair() != m_crosshair || !m_init)
      {
        setCrosshair(m_view->crosshair(), time);
      }

      if (!m_actorsInUse)
      {
        for (auto i : { 0, 1, 2 })
        {
          m_view->addActor(m_actors[i]);
        }

        m_actorsInUse = true;
      }
    }
    else
    {
      if (m_actorsInUse)
      {
        for (auto i : { 0, 1, 2 })
        {
          m_view->removeActor(m_actors[i]);
        }

        m_actorsInUse = false;
      }
    }

    //TODO setRenderRequired(requiresRender() || needsRender);
  }

} // namespace ESPINA
