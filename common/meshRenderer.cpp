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


#include "meshRenderer.h"
#include "products.h"

// Para View
#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqPipelineRepresentation.h>
#include <vtkSMProxy.h>
#include <pqServer.h>
#include <pqScalarsToColors.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMProxyProperty.h>
#include <pqLookupTableManager.h>

MeshRenderer::MeshRenderer(QWidget* parent)
: IViewWidget(parent)
{

}


void MeshRenderer::updateState(bool checked)
{

}

IViewWidget* MeshRenderer::clone()
{

}

void MeshRenderer::renderInView(QModelIndex index, pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();

  for (int row = 0; row < index.model()->rowCount(index); row++)
  {
    IModelItem *item = static_cast<IModelItem *>(index.child(row,0).internalPointer());
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    
    seg->representation("Mesh")->render(view);
  }
}

