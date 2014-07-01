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

// EspINA
#include "RectangularRegionSliceSelector.h"
#include "RectangularRegion.h"
#include <GUI/View/View2D.h>

// Qt
#include <QPushButton>

using namespace EspINA;

//----------------------------------------------------------------------------
RectangularRegionSliceSelector::RectangularRegionSliceSelector(RectangularRegion* region)
: m_region(region)
, m_leftWidget  (new QPushButton())
, m_rightWidget    (new QPushButton())
{
  m_leftWidget->setMaximumHeight(20);
  m_rightWidget->setMaximumHeight(20);

  connect(region, SIGNAL(modified(double*)),
          this, SLOT(update()));
  connect(m_leftWidget, SIGNAL(clicked(bool)),
          this, SLOT(leftWidgetClicked()));
  connect(m_rightWidget, SIGNAL(clicked(bool)),
          this, SLOT(rightWidgetClicked()));
  update();
}

//----------------------------------------------------------------------------
RectangularRegionSliceSelector::~RectangularRegionSliceSelector()
{
  delete m_leftWidget;
  delete m_rightWidget;
}


//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::setPlane(const Plane plane)
{
  SliceSelectorWidget::setPlane(plane);
  update();
}

//----------------------------------------------------------------------------
QWidget* RectangularRegionSliceSelector::leftWidget() const
{
  return m_leftWidget;
}

//----------------------------------------------------------------------------
QWidget* RectangularRegionSliceSelector::rightWidget() const
{
  return m_rightWidget;
}

//----------------------------------------------------------------------------
SliceSelectorWidget* RectangularRegionSliceSelector::clone()
{
  RectangularRegionSliceSelector *selector;
  selector = new RectangularRegionSliceSelector(m_region);

  selector->m_leftLabel  = m_leftLabel;
  selector->m_rightLabel = m_rightLabel;
  selector->m_plane      = m_plane;
  selector->m_view       = m_view;
  return selector;
}


//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::update()
{
  auto bounds = m_region->bounds();

  int i = normalCoordinateIndex(m_plane);
  m_leftWidget->setText(QString("%1(%2)")
                               .arg(m_leftLabel)
                               .arg(bounds[2*i]));
  m_rightWidget->setText(QString("%1(%2)")
                                .arg(m_rightLabel)
                                .arg(bounds[2*i+1]));
}

//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::leftWidgetClicked()
{
  if (m_view)
  {
    auto bounds = m_region->bounds();

    int i = normalCoordinateIndex(m_plane);

    bounds[2*i] = m_view->slicingPosition();

    if (bounds[2*i] > bounds[2*i+1])
      std::swap(bounds[2*i],bounds[2*i+1]);


    m_region->setBounds(Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]});
    update();
  }
}

//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::rightWidgetClicked()
{
  if (m_view)
  {
    auto bounds = m_region->bounds();

    int i = normalCoordinateIndex(m_plane);

    bounds[2*i+1] = m_view->slicingPosition();

    if (bounds[2*i] > bounds[2*i+1])
      std::swap(bounds[2*i],bounds[2*i+1]);

    m_region->setBounds(Bounds{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]});
    update();
  }
}
