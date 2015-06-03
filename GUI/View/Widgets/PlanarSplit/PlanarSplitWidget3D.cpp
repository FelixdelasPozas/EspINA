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
#include <GUI/View/RenderView.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget3D.h>

// VTK
#include <vtkImplicitPlaneWidget2.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkMath.h>
#include <vtkRenderWindow.h>
#include <vtkWidgetRepresentation.h>
#include <vtkImplicitPlaneRepresentation.h>

using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
PlanarSplitWidget3D::PlanarSplitWidget3D(PlanarSplitEventHandler *handler)
: PlanarSplitWidget{handler}
, m_widget         {vtkSmartPointer<vtkImplicitPlaneWidget2>::New()}
, m_command        {vtkSmartPointer<vtkSplitCommand>::New()}
, m_view           {nullptr}
{
  m_command->setWidget(this);
}

//-----------------------------------------------------------------------------
PlanarSplitWidget3D::~PlanarSplitWidget3D()
{
  if(m_widget->GetEnabled())
  {
    uninitializeImplementation();
  }
}

//-----------------------------------------------------------------------------
TemporalRepresentation3DSPtr PlanarSplitWidget3D::clone()
{
  return std::make_shared<PlanarSplitWidget3D>(m_handler);
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget3D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget3D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return false;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget3D::initializeImplementation(RenderView* view)
{
  m_widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
  m_widget->CreateDefaultRepresentation();

  m_view = view;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget3D::uninitializeImplementation()
{
  m_widget->RemoveObserver(m_command);

  m_view = nullptr;
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* PlanarSplitWidget3D::vtkWidget()
{
  return m_widget;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget3D::setPlanePoints(vtkSmartPointer<vtkPoints> points)
{
  double point1[3], point2[3], upVector[3], normal[3];
  points->GetPoint(0, point1);
  points->GetPoint(1, point2);
  points->GetPoint(2, upVector);
  double vector[3] = { point2[0] - point1[0], point2[1] - point1[1], point2[2] - point1[2] };

  vtkMath::Cross(vector, upVector, normal);
  auto rep = static_cast<vtkImplicitPlaneRepresentation*>(m_widget->GetRepresentation());
  rep->SetNormal(normal);
  rep->SetOrigin(point1);
  rep->Modified();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPlane> PlanarSplitWidget3D::getImplicitPlane(const NmVector3& spacing) const
{
  auto plane = vtkSmartPointer<vtkPlane>::New();
  auto rep = static_cast<vtkImplicitPlaneRepresentation*>(m_widget->GetRepresentation());
  rep->GetPlane(plane);

  return plane;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget3D::setSegmentationBounds(const Bounds& bounds)
{
  double dBounds[6]{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};

  auto rep = static_cast<vtkImplicitPlaneRepresentation*>(m_widget->GetRepresentation());
  rep->SetPlaceFactor(1);
  rep->PlaceWidget(dBounds);
  rep->SetOrigin((bounds[0] + bounds[1]) / 2, (bounds[2] + bounds[3]) / 2, (bounds[4] + bounds[5]) / 2);
  rep->UpdatePlacement();
  rep->OutlineTranslationOff();
  rep->ScaleEnabledOff();
  rep->OutsideBoundsOff();
  rep->SetVisibility(true);
  rep->Modified();
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitWidget3D::getPlanePoints() const
{
  return vtkSmartPointer<vtkPoints>::New();
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget3D::planeIsValid() const
{
  return true;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget3D::disableWidget()
{
  if(m_view)
  {
    m_widget->Off();
    m_view->refresh();
  }
}
