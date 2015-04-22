/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "OrthogonalRegionSliceSelector.h"
#include "OrthogonalRegion.h"
#include <GUI/View/View2D.h>

// Qt
#include <QPushButton>

using namespace ESPINA;

//----------------------------------------------------------------------------
OrthogonalRegionSliceSelector::OrthogonalRegionSliceSelector(OrthogonalRegion* region)
: m_region     {region}
, m_leftWidget {RenderView::createButton(":/espina/from_slice.svg", "")}
, m_rightWidget{RenderView::createButton(":/espina/to_slice.svg",   "")}
{
  m_leftWidget ->setEnabled(true);
  m_rightWidget->setEnabled(true);

  connect(region, SIGNAL(modified(Bounds)),
          this, SLOT(update()));
  connect(m_leftWidget, SIGNAL(clicked(bool)),
          this, SLOT(leftWidgetClicked()));
  connect(m_rightWidget, SIGNAL(clicked(bool)),
          this, SLOT(rightWidgetClicked()));

  update();
}

//----------------------------------------------------------------------------
OrthogonalRegionSliceSelector::~OrthogonalRegionSliceSelector()
{
  delete m_leftWidget;
  delete m_rightWidget;
}


//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::setView(View2D* view)
{
  SliceSelector::setView(view);
  if (m_view)
  {
    connect(m_view, SIGNAL(crosshairPlaneChanged(Plane,Nm)),
            this,   SLOT(update()));

    update();
  }
}

//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::setPlane(const Plane plane)
{
  SliceSelector::setPlane(plane);
  update();
}

//----------------------------------------------------------------------------
QWidget* OrthogonalRegionSliceSelector::leftWidget() const
{
  return m_leftWidget;
}

//----------------------------------------------------------------------------
QWidget* OrthogonalRegionSliceSelector::rightWidget() const
{
  return m_rightWidget;
}

//----------------------------------------------------------------------------
SliceSelectorSPtr OrthogonalRegionSliceSelector::clone()
{
  auto selector = std::make_shared<OrthogonalRegionSliceSelector>(m_region);

  selector->m_label = m_label;
  selector->m_plane = m_plane;
  selector->m_view  = m_view;

  return selector;
}


//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::update()
{
  if (m_view)
  {
    auto bounds = m_region->bounds();

    int i = normalCoordinateIndex(m_plane);

    Q_ASSERT(false);
    // TODO use coordinate system
//     Nm voxelSize     = m_view->slicingStep()[i];
//     Nm lowerDistance = m_view->slicingPosition() - bounds[2*i];
//     Nm upperDistance = bounds[2*i+1] - m_view->slicingPosition();
//
//     m_leftWidget ->setEnabled(lowerDistance < 0 || lowerDistance > voxelSize);
//     m_rightWidget->setEnabled(upperDistance < 0 || upperDistance > voxelSize);
//
//     Nm leftSlicePosition  = m_view->slicingPosition() - voxelSize/2;
//     Nm rightSlicePosition = m_view->slicingPosition() + voxelSize/2;
//
//     QString tooltip = tr("<b>%1</b><br>Place %2 at %3 nm").arg(m_label);
//     m_leftWidget ->setToolTip(tooltip.arg(leftFaceLabel()) .arg(leftSlicePosition));
//     m_rightWidget->setToolTip(tooltip.arg(rightFaceLabel()).arg(rightSlicePosition));
  }
}

//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::leftWidgetClicked()
{
  moveEdge(Lower);
}

//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::rightWidgetClicked()
{
  moveEdge(Upper);
}

//----------------------------------------------------------------------------
void OrthogonalRegionSliceSelector::moveEdge(Edge edge)
{
  if (m_view)
  {
    auto bounds = m_region->bounds();

    int i   = normalCoordinateIndex(m_plane);
    Nm sign = (Lower==edge)?-0.5:0.5;

    Q_ASSERT(false);
    //TODO bounds[2*i+edge] = m_view->slicingPosition() + sign*m_view->slicingStep()[i];

    if (bounds[2*i] > bounds[2*i+1])
    {
      std::swap(bounds[2*i],bounds[2*i+1]);
    }

    m_region->setBounds(bounds);

    update();
  }
}

//----------------------------------------------------------------------------
QString OrthogonalRegionSliceSelector::leftFaceLabel() const
{
  switch (m_plane)
  {
    case Plane::XY:
      return tr("back face");
    case Plane::XZ:
      return tr("top face");
    case Plane::YZ:
      return tr("left face");
    default:
      return tr("face");
  }
}

//----------------------------------------------------------------------------
QString OrthogonalRegionSliceSelector::rightFaceLabel() const
{
  switch (m_plane)
  {
    case Plane::XY:
      return tr("front face");
    case Plane::XZ:
      return tr("bottom face");
    case Plane::YZ:
      return tr("right face");
    default:
      return tr("face");
  }
}
