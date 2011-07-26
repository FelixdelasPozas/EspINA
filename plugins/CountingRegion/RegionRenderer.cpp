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

#include "sample.h"

#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>
#include <espina.h>

#include <assert.h>
#include <QDebug>
#include "CountingRegion.h"

RegionRenderer::RegionRenderer(QWidget* parent)
: IViewWidget(parent)
{
  setIcon(QIcon(":/espina/applyCR"));
}

IViewWidget* RegionRenderer::clone()
{
  return new RegionRenderer();
}


void RegionRenderer::updateState(bool checked)
{
  //setIcon(checked?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
}

void RegionRenderer::renderInView(QModelIndex index, pqView* view)
{
  if (!index.isValid())
    return;
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
 
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  Sample *sample = dynamic_cast<Sample *>(item);
  if (sample)
    sample->representation("BoundingRegion")->render(view);

  for (int row = 0; row < index.model()->rowCount(index); row++)
  {
    renderInView(index.child(row,0),view);
  }
}

