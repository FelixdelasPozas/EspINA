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
#include <common/undo/AddFilter.h>
#include <common/undo/AddSegmentation.h>

static const QString SEGMHA = "segmha";

//-----------------------------------------------------------------------------
SegmhaImporter::SegmhaImporter(QObject* parent)
{
}

void SegmhaImporter::onStartup()
{
  // Register filter and reader factories
//   manager->registerFilter(filter,this);
  EspinaFactory::instance()->registerReader(SEGMHA, this);
}

//-----------------------------------------------------------------------------
FilterPtr SegmhaImporter::createFilter(const QString filter, const QString args)
{
//   if (filter == SFRF)
//   {
//     SegmhaImporterFilter *sr_sif = new SegmhaImporterFilter(args);
//     return sr_sif;
//   }
  qWarning("::createFilter: Error no such a Filter");
  return FilterPtr(NULL);
}

//-----------------------------------------------------------------------------
void SegmhaImporter::readFile(const QString file)
{
  Q_ASSERT(File::extension(file) == SEGMHA);

  QSharedPointer<SegmhaImporterFilter> filter =
   QSharedPointer<SegmhaImporterFilter>(new SegmhaImporterFilter(file));

  Q_ASSERT(filter->numProducts() > 0);

  QList<SegmentationPtr> segs = filter->segmentations();
//   seg = EspinaFactory::instance()->createSegmentation(this, segImage->data(0));

  QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
  undo->beginMacro("Import Segmha");
  undo->push(new AddFilter(filter));
//   undo->push(new AddRelation(input,filter.data(),"Channel"));
  foreach(SegmentationPtr seg, segs)
  {
    undo->push(new AddSegmentation(seg));
  }
//   undo->push(new AddRelation(filter, seg, "CreateSegmentation"));
//   undo->push(new AddRelation(sample, seg.data(), "where"));
  undo->endMacro();
}
