/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "RectangularRegionSliceSelector.h"
#include "RectangularRegion.h"
#include <SliceView.h>

#include <QPushButton>

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
void RectangularRegionSliceSelector::setPlane(const PlaneType plane)
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
  Nm bounds[6];
  m_region->bounds(bounds);

  m_leftWidget->setText(QString("%1(%2)")
                               .arg(m_leftLabel)
                               .arg(bounds[2*m_plane  ]));
  m_rightWidget->setText(QString("%1(%2)")
                                .arg(m_rightLabel)
                                .arg(bounds[2*m_plane+1]));
}

//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::leftWidgetClicked()
{
  if (m_view)
  {
    Nm bounds[6];
    m_region->bounds(bounds);

    bounds[2*m_plane] = m_view->slicingPosition();

    if (bounds[2*m_plane] > bounds[2*m_plane+1])
      std::swap(bounds[2*m_plane],bounds[2*m_plane+1]);

    m_region->setBounds(bounds);
    update();
  }
}

//----------------------------------------------------------------------------
void RectangularRegionSliceSelector::rightWidgetClicked()
{
  if (m_view)
  {
    Nm bounds[6];
    m_region->bounds(bounds);

    bounds[2*m_plane+1] = m_view->slicingPosition();

    if (bounds[2*m_plane] > bounds[2*m_plane+1])
      std::swap(bounds[2*m_plane],bounds[2*m_plane+1]);

    m_region->setBounds(bounds);
    update();
  }

}
