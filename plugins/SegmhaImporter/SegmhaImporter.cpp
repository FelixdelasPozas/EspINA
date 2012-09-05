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

#include <QDebug>
#include "SegmhaImporterFilter.h"
#include <common/EspinaCore.h>
#include <QFileDialog>
#include <common/undo/AddSample.h>
#include <common/undo/AddChannel.h>
#include <common/undo/AddRelation.h>
#include <common/model/Segmentation.h>
#include <common/selection/SelectionManager.h>
#include <QApplication>

const QString INPUTLINK = "Input";

//-----------------------------------------------------------------------------
SegmhaImporter::UndoCommand::UndoCommand(Sample* sample, Channel* channel, SegmhaImporterFilter* filter)
: m_sample (sample)
, m_channel(channel)
, m_filter(filter)
{
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::redo()
{
  EspinaCore    *espina  = EspinaCore::instance();
  EspinaFactory *factory = EspinaFactory::instance();
  QSharedPointer<EspinaModel> model(espina->model());

  // update taxonomy
  espina->model()->setTaxonomy(m_filter->taxonomy());

  if (m_segs.isEmpty())
  {
    Segmentation *seg;
    for (int i=0; i < m_filter->numberOutputs(); i++)
    {
      seg = factory->createSegmentation(m_filter, i);
      m_filter->initSegmentation(seg, i);
      m_segs << seg;
    }
  }

  model->addFilter(m_filter);
  model->addSegmentation(m_segs);

  model->addRelation(m_channel, m_filter, Channel::LINK);
  foreach(Segmentation *seg, m_segs)
  {
    model->addRelation(m_filter, seg, CREATELINK);
    model->addRelation(m_sample, seg, "where");
    model->addRelation(m_channel, seg, Channel::LINK);
    seg->initializeExtensions();
  }
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::undo()
{
//   QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());
//
//   QList<Segmentation *> segs = m_filter->segmentations();
//
//
//   model->removeRelation(m_channel, m_filter, "Channel");
//   foreach(Segmentation *seg, segs)
//   {
//     model->removeRelation(m_filter, seg, CREATELINK);
//     model->removeRelation(m_sample, seg, "where");
//     model->removeRelation(m_channel, seg, "Channel");
//     model->removeSegmentation(seg);
//   }
//   model->removeFilter(m_filter);
}

static const QString SEGMHA = "segmha";

//-----------------------------------------------------------------------------
SegmhaImporter::SegmhaImporter()
{
  // Register filter and reader factories
  QStringList supportedExtensions;
  supportedExtensions << SEGMHA;
  EspinaFactory::instance()->registerReaderFactory(this,
						   SegmhaImporterFilter::SUPPORTED_FILES,
						   supportedExtensions);
  EspinaFactory::instance()->registerFilter(SegmhaImporterFilter::TYPE, this);
}

//-----------------------------------------------------------------------------
Filter* SegmhaImporter::createFilter(const QString filter,
				     Filter::NamedInputs inputs,
				     const ModelItem::Arguments args)
{
  Q_ASSERT(SegmhaImporterFilter::TYPE == filter);

  return new SegmhaImporterFilter(inputs, args);
}

//-----------------------------------------------------------------------------
bool SegmhaImporter::readFile(const QFileInfo file)
{
  qDebug() << file.absoluteFilePath();
  Q_ASSERT(SEGMHA == file.suffix());
  EspinaCore *espina = EspinaCore::instance();

  QSharedPointer<QUndoStack> undo(espina->undoStack());
  undo->beginMacro("Import Segmha");

  if (!espina->sample())
  {
    QFileDialog fileDialog;
    fileDialog.setObjectName("SelectChannelFile");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select channel file for %1:").arg(file.fileName()));
    fileDialog.setDirectory(file.dir());
    fileDialog.setFilter(CHANNEL_FILES);

    if (fileDialog.exec() != QDialog::Accepted)
      return false;

    if (!espina->loadFile(fileDialog.selectedFiles().first()))
      return false;
  }

  Filter::NamedInputs inputs;
  Filter::Arguments args;
  args[SegmhaImporterFilter::FILE] = file.absoluteFilePath();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  SegmhaImporterFilter *filter = new SegmhaImporterFilter(inputs, args);
  filter->update();
  if (filter->numberOutputs() == 0)
  {
    delete filter;
    return false;
  }
  undo->push(new UndoCommand(espina->sample(), SelectionManager::instance()->activeChannel(), filter));
  undo->endMacro();
  QApplication::restoreOverrideCursor();

  return true;
}

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, SegmhaImporter)

