/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "AddSegmentation.h"

#include <EspinaCore.h>
#include <model/Segmentation.h>
#include <model/EspinaModel.h>
#include <processing/Filter.h>


AddSegmentation::AddSegmentation(QSharedPointer<Filter> filter,
				 int element,
				 QUndoCommand* parent)
: QUndoCommand(parent)
, m_filter(filter)
, m_productIndex(element)
, m_segmentation(NULL)
{
}


void AddSegmentation::redo()
{
  QSharedPointer<EspinaModel>  model = EspinaCore::instance()->model();

  Q_ASSERT(m_segmentation == NULL);
  m_segmentation = new Segmentation(m_filter, m_filter->product(m_productIndex));
  model->addSegmentation(m_segmentation);
}

void AddSegmentation::undo()
{
  QSharedPointer<EspinaModel>  model = EspinaCore::instance()->model();

  model->removeSegmentation(m_segmentation);
  delete m_segmentation;
  m_segmentation = NULL;
}
