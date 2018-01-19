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
#include "GUI/View/Widgets/OrthogonalRegion/OrthogonalSliceSelector.h"
#include "OrthogonalRepresentation.h"
#include <GUI/View/View2D.h>

// Qt
#include <QPushButton>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

//----------------------------------------------------------------------------
OrthogonalSliceSelector::OrthogonalSliceSelector(OrthogonalRepresentationSPtr region, QWidget *parent)
: m_representation(region)
, m_view          {nullptr}
, m_plane         {Plane::UNDEFINED}
, m_lowerWidget   {RenderView::createButton(":/espina/from_slice.svg", "Set lower slice of range", parent)}
, m_upperWidget   {RenderView::createButton(":/espina/to_slice.svg",   "Set upper slice of range", parent)}
{
}

//----------------------------------------------------------------------------
OrthogonalSliceSelector::OrthogonalSliceSelector(OrthogonalSliceSelector &selector, QWidget *parent)
: m_representation(selector.m_representation)
, m_view          {selector.m_view}
, m_plane         {selector.m_plane}
, m_lowerWidget   {RenderView::createButton(":/espina/from_slice.svg", "Set lower slice of range", parent)}
, m_upperWidget   {RenderView::createButton(":/espina/to_slice.svg",   "Set upper slice of range", parent)}
, m_label         {selector.m_label}
{
}

//----------------------------------------------------------------------------
OrthogonalSliceSelector::~OrthogonalSliceSelector()
{
  delete m_lowerWidget;
  delete m_upperWidget;
}

//----------------------------------------------------------------------------
QWidget* OrthogonalSliceSelector::lowerWidget() const
{
  return m_lowerWidget;
}

//----------------------------------------------------------------------------
QWidget* OrthogonalSliceSelector::upperWidget() const
{
  return m_upperWidget;
}

//----------------------------------------------------------------------------
SliceSelectorSPtr OrthogonalSliceSelector::clone(RenderView *view, Plane plane)
{
  auto selector = std::make_shared<OrthogonalSliceSelector>(*this, view);

  selector->m_view  = view;
  selector->m_plane = plane;

  connect(selector->m_view, SIGNAL(crosshairPlaneChanged(Plane, Nm)),
          selector.get(),   SLOT(update()));

  connect(selector->m_representation.get(), SIGNAL(boundsChanged(Bounds)),
          selector.get(),                   SLOT(update()));

  connect(selector->m_lowerWidget, SIGNAL(clicked(bool)),
          selector.get(),          SLOT(lowerWidgetClicked()));

  connect(selector->m_upperWidget, SIGNAL(clicked(bool)),
          selector.get(),          SLOT(upperWidgetClicked()));

  selector->update();

  return selector;
}


//----------------------------------------------------------------------------
void OrthogonalSliceSelector::update()
{
  if(m_view)
  {
    auto bounds = m_representation->bounds();

    int i = normalIndex();

    Nm lowerDistance = voxelCenter() - bounds[2*i];
    Nm upperDistance = bounds[2*i+1] - voxelCenter();

    m_lowerWidget->setEnabled(lowerDistance < 0 || lowerDistance > voxelSize());
    m_upperWidget->setEnabled(upperDistance < 0 || upperDistance > voxelSize());

    updateLabel();
  }
}

//----------------------------------------------------------------------------
void OrthogonalSliceSelector::lowerWidgetClicked()
{
  moveEdge(Lower);
}

//----------------------------------------------------------------------------
void OrthogonalSliceSelector::upperWidgetClicked()
{
  moveEdge(Upper);
}

//----------------------------------------------------------------------------
void OrthogonalSliceSelector::moveEdge(Edge edge)
{
  Q_ASSERT(m_view);

  auto bounds = m_representation->bounds();

  int i = normalIndex();

  bounds[2*i+edge] = (Lower == edge)?lowerSlice():upperSlice();

  if (bounds[2*i] > bounds[2*i+1])
  {
    std::swap(bounds[2*i],bounds[2*i+1]);
  }

  m_representation->setBounds(bounds);

  update();
}

//----------------------------------------------------------------------------
QString OrthogonalSliceSelector::lowerLabel() const
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
QString OrthogonalSliceSelector::upperLabel() const
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

//----------------------------------------------------------------------------
int OrthogonalSliceSelector::normalIndex() const
{
  return normalCoordinateIndex(m_plane);
}

//----------------------------------------------------------------------------
Nm OrthogonalSliceSelector::voxelCenter() const
{
  return m_view->crosshair()[normalIndex()];
}

//----------------------------------------------------------------------------
Nm OrthogonalSliceSelector::voxelSize() const
{
  return m_view->sceneResolution()[normalIndex()];
}

//----------------------------------------------------------------------------
Nm OrthogonalSliceSelector::halfVoxelSize() const
{
  return voxelSize()/2;
}

//----------------------------------------------------------------------------
Nm OrthogonalSliceSelector::lowerSlice() const
{
  return voxelCenter() - halfVoxelSize();
}

//----------------------------------------------------------------------------
Nm OrthogonalSliceSelector::upperSlice() const
{
  return voxelCenter() + halfVoxelSize();
}

//----------------------------------------------------------------------------
void OrthogonalSliceSelector::updateLabel()
{
  QString tooltip = tr("<b>%1</b><br>Place %2 at %3 nm").arg(m_label);

  m_lowerWidget->setToolTip(tooltip.arg(lowerLabel()).arg(lowerSlice()));
  m_upperWidget->setToolTip(tooltip.arg(upperLabel()).arg(upperSlice()));
}
