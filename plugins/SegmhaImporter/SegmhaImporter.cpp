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
#include "SegmhaImporterFilter.h"

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
//   qWarning("::createFilter: Error no such a Filter");
//   return NULL;
}

//-----------------------------------------------------------------------------
void SegmhaImporter::readFile(const QString file)
{
  const QString extension = file.section('.',-1);
  Q_ASSERT(extension == SEGMHA);

//     proxy->updatePipeline();
//     //TODO: How to manage these kind of filters
  SegmhaImporterFilter *segmhaImporter = new SegmhaImporterFilter(file.section('/', -1));
}
