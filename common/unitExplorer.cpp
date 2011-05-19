/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "unitExplorer.h"

#include "espina.h"
#include "products.h"

UnitExplorer::UnitExplorer(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  setupUi(this);
  
}


void UnitExplorer::setVisible(bool visible)
{
  QWidget::setVisible(visible);
  
  double spacing[3];
  
  if (!EspINA::instance()->activeSample())
    return;
  
  EspINA::instance()->activeSample()->spacing(spacing);
  
  m_xDist->setValue(spacing[0]);
  m_yDist->setValue(spacing[1]);
  m_zDist->setValue(spacing[2]);
}

