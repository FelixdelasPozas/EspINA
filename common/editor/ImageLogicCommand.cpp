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


#include "ImageLogicCommand.h"

#include <EspinaCore.h>

ImageLogicCommand::ImageLogicCommand(QList< Segmentation* > inputs,
				     ImageLogicFilter::Operation op)
: m_input(inputs)
{
  m_filter = new ImageLogicFilter(inputs, op);
}

void ImageLogicCommand::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->removeSegmentation(m_input);
  model->addFilter(m_filter);
  Segmentation *seg = m_filter->product(0);
  seg->setTaxonomy(EspinaCore::instance()->activeTaxonomy());
  model->addSegmentation(seg);
//   model->addRelation(m_channel, m_filter, "Channel");
//   m_seg->setTaxonomy(m_taxonomy);
//   model->addSegmentation(m_seg);
//   model->addRelation(m_filter, m_seg, "CreateSegmentation");
//   model->addRelation(m_sample, m_seg, "where");
//   model->addRelation(m_channel, m_seg, "Channel");
//   m_seg->initialize();
}

void ImageLogicCommand::undo()
{
  QUndoCommand::undo();
}
