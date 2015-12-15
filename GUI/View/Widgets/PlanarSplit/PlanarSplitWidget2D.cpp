/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/View/View2D.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget2D.h>
#include <GUI/View/Widgets/PlanarSplit/vtkPlanarSplitWidget.h>
#include <GUI/Representations/Frame.h>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkPoints.h>
#include <vtkPlane.h>
#include <vtkMath.h>
#include <vtkRenderWindow.h>

using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
PlanarSplitWidget2D::PlanarSplitWidget2D(PlanarSplitEventHandler *handler)
: PlanarSplitWidget{handler}
, m_widget         {vtkSmartPointer<vtkPlanarSplitWidget>::New()}
, m_command        {vtkSmartPointer<vtkSplitCommand>::New()}
, m_view           {nullptr}
{
  m_command->setWidget(this);
}

//-----------------------------------------------------------------------------
PlanarSplitWidget2D::~PlanarSplitWidget2D()
{
  if(m_widget->GetEnabled())
  {
    uninitializeImplementation();
  }
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::setPlanePoints(vtkSmartPointer<vtkPoints> points)
{
  m_widget->setPoints(points);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitWidget2D::getPlanePoints() const
{
  double upVector[3]{0,0,0};
  upVector[normalCoordinateIndex(m_widget->getPlane())] = 1;

  auto points = m_widget->getPoints();
  points->InsertNextPoint(upVector);

  return points;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::setSegmentationBounds(const Bounds &bounds)
{
  double dBounds[6]{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};

  m_widget->setSegmentationBounds(dBounds);
  m_widget->SetEnabled(true);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::setPlane(Plane plane)
{
  m_widget->setPlane(plane);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::setRepresentationDepth(Nm depth)
{
}

//-----------------------------------------------------------------------------
TemporalRepresentation2DSPtr PlanarSplitWidget2D::clone()
{
  return std::make_shared<PlanarSplitWidget2D>(m_handler);
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  auto idx = normalCoordinateIndex(m_widget->getPlane());
  return m_widget->slice() != crosshair[idx];
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget2D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget2D::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget2D::acceptInvalidationFrame(const Representations::FrameCSPtr frame) const
{
  return false;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::initializeImplementation(RenderView* view)
{
  auto view2D = view2D_cast(view);

  m_widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
  m_widget->setPlane(view2D->plane());
  m_widget->setRepresentationDepth(view2D->widgetDepth());
  m_widget->setSlice(view2D->crosshair()[normalCoordinateIndex(view2D->plane())]);
  m_widget->CreateDefaultRepresentation();

  m_view = view;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::uninitializeImplementation()
{
  m_widget->RemoveObserver(m_command);
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* PlanarSplitWidget2D::vtkWidget()
{
  return m_widget;
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget2D::planeIsValid() const
{
  vtkSmartPointer<vtkPoints> points = this->getPlanePoints();
  double point1[3], point2[3];
  points->GetPoint(0, point1);
  points->GetPoint(1, point2);

  return ((point1[0] != point2[0]) || (point1[1] != point2[1]) || (point1[2] != point2[2]));
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPlane> PlanarSplitWidget2D::getImplicitPlane(const NmVector3 &spacing) const
{
  vtkSmartPointer<vtkPlane> plane = nullptr;

  if (!this->planeIsValid()) return plane;

  plane = vtkSmartPointer<vtkPlane>::New();

  vtkSmartPointer<vtkPoints> points = getPlanePoints();
  double point1[3], point2[3];
  points->GetPoint(0, point1);
  points->GetPoint(1, point2);

  double normal[3], upVector[3];
  double planeVector[3] = { point2[0]-point1[0] - spacing[0]/2,
                            point2[1]-point1[1] - spacing[1]/2,
                            point2[2]-point1[2] - spacing[2]/2};

  switch (m_widget->getPlane())
  {
    case Plane::XY:
      upVector[0] = upVector[1] = 0;
      upVector[2] = 1;
      break;
    case Plane::XZ:
      upVector[0] = upVector[2] = 0;
      upVector[1] = 1;
      break;
    case Plane::YZ:
      upVector[1] = upVector[2] = 0;
      upVector[0] = 1;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  vtkMath::Cross(planeVector, upVector, normal);
  plane->SetOrigin(point1);
  plane->SetNormal(normal);
  plane->Modified();

  return plane;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::display(const ESPINA::GUI::Representations::FrameCSPtr& frame)
{
  if(m_widget->getPlane() == Plane::UNDEFINED || m_widget->GetEnabled() == false) return;

  auto idx = normalCoordinateIndex(m_widget->getPlane());
  m_widget->setSlice(frame->crosshair[idx]);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget2D::disableWidget()
{
  if(m_view)
  {
    m_widget->disableWidget();
    m_view->refresh();
  }
}
