/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "Crosshairs.h"
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>

Crosshairs::Crosshairs(QWidget* parent): IViewWidget(parent)
{
  setIcon(QIcon(":/espina/hidePlanes"));
}


void Crosshairs::renderInView(pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  dp->setRepresentationVisibility((*m_planes[0])->getOutputPort(0),view,true);
  dp->setRepresentationVisibility((*m_planes[1])->getOutputPort(0),view,true);
  dp->setRepresentationVisibility((*m_planes[2])->getOutputPort(0),view,true);
}

void Crosshairs::updateState(bool checked)
{
  setIcon(checked?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
}
