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


#include "RegionRenderer.h"

#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>
#include <espina.h>

#include <assert.h>
#include <QDebug>

RegionRenderer::RegionRenderer(QMap< Sample*, QList< pqPipelineSource* > >& regions, QWidget* parent)
: IViewWidget(parent)
, m_regions(regions)
{
  assert(m_regions.size() == regions.size());
  setIcon(QIcon(":/espina/applyCR"));
}

IViewWidget* RegionRenderer::clone()
{
  return new RegionRenderer(m_regions);
}


void RegionRenderer::updateState(bool checked)
{
  //setIcon(checked?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
}

void RegionRenderer::renderInView(pqView* view)
{
  qDebug() << "Regions to be painted" << m_regions[EspINA::instance()->activeSample()].size();
  foreach (pqPipelineSource *region, m_regions[EspINA::instance()->activeSample()])
  {
    pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
    dp->setRepresentationVisibility(region->getOutputPort(0),view,isChecked());
  }
}

