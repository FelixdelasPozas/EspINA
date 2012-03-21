/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "SegmhaImporter.h"

#include <common/model/EspinaFactory.h>
#include <common/File.h>

#include "SegmhaImporterFilter.h"
#include <common/EspinaCore.h>
#include <common/model/Channel.h>

static const QString SEGMHA = "segmha";

//-----------------------------------------------------------------------------
SegmhaImporter::UndoCommand::UndoCommand(SegmhaImporterFilter *filter)
: m_filter(filter)
{
  m_channel = m_filter->channel();
  ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN, "mark");
  Q_ASSERT(samples.size() > 0);
  m_sample = dynamic_cast<Sample *>(samples.first());
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  QList<Segmentation *> segs = m_filter->segmentations();

  model->addFilter(m_filter);
  model->addSegmentation(segs);

  model->addRelation(m_channel, m_filter, "Channel");
  foreach(Segmentation *seg, segs)
  {
    model->addRelation(m_filter, seg, "CreateSegmentation");
    model->addRelation(m_sample, seg, "where");
  }
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  QList<Segmentation *> segs = m_filter->segmentations();


  model->removeRelation(m_channel, m_filter, "Channel");
  foreach(Segmentation *seg, segs)
  {
    model->removeRelation(m_filter, seg, "CreateSegmentation");
    model->removeRelation(m_sample, seg, "where");
    model->removeSegmentation(seg);
  }
  model->removeFilter(m_filter);
}


//-----------------------------------------------------------------------------
SegmhaImporter::SegmhaImporter(QObject* parent)
{
}

void SegmhaImporter::onStartup()
{
  // Register filter and reader factories
  EspinaFactory::instance()->registerReader(SEGMHA, this);
  EspinaFactory::instance()->registerFilter(SIF, this);
}

//-----------------------------------------------------------------------------
Filter *SegmhaImporter::createFilter(const QString filter, const ModelItem::Arguments args)
{
  Q_ASSERT(filter == SIF);

  return new SegmhaImporterFilter(args);
}

//-----------------------------------------------------------------------------
void SegmhaImporter::readFile(const QString file)
{
  Q_ASSERT(File::extension(file) == SEGMHA);

  SegmhaImporterFilter *filter = new SegmhaImporterFilter(file);
  Q_ASSERT(filter->numProducts() > 0);

  QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
  undo->beginMacro("Import Segmha");
  undo->push(new UndoCommand(filter));
  undo->endMacro();
}
