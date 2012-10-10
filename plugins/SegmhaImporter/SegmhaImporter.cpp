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

#include "SegmhaImporterFilter.h"

#include <common/model/EspinaFactory.h>
#include <common/undo/AddSample.h>
#include <common/undo/AddChannel.h>
#include <common/undo/AddRelation.h>
#include <common/model/Segmentation.h>
#include <common/model/EspinaModel.h>
#include <common/IO/EspinaIO.h>

#include <QApplication>
#include <QFileDialog>
#include <QDebug>

const QString INPUTLINK = "Input";

//-----------------------------------------------------------------------------
SegmhaImporter::UndoCommand::UndoCommand(Sample* sample,
                                         Channel* channel,
                                         SegmhaImporterFilter* filter,
                                         EspinaModel *model)
: m_model(model)
, m_sample (sample)
, m_channel(channel)
, m_filter(filter)
{
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::redo()
{
  // update taxonomy
  m_model->setTaxonomy(m_filter->taxonomy());

  if (m_segs.isEmpty())
  {
    Segmentation *seg;
    for (int i=0; i < m_filter->numberOutputs(); i++)
    {
      seg = m_model->factory()->createSegmentation(m_filter, i);
      m_filter->initSegmentation(seg, i);
      m_segs << seg;
    }
  }

  ModelItemExtension *countingRegionExt = m_channel->extension("CountingRegionExtension");
  if (countingRegionExt)
  {
    Nm inclusive[3], exclusive[3];
    m_filter->countingRegion(inclusive, exclusive);
    EspinaVolume::SpacingType spacing = m_channel->itkVolume()->GetSpacing();
    for(int i=0; i<3;i++)
    {
      inclusive[i] = inclusive[i]*spacing[i];
      exclusive[i] = exclusive[i]*spacing[i];
    }

    QString rcb = QString("RectangularBoundingRegion=%1,%2,%3,%4,%5,%6;")
      .arg(inclusive[0]).arg(inclusive[1]).arg(inclusive[2])
      .arg(exclusive[0]).arg(exclusive[1]).arg(exclusive[2]);
    //qDebug() << "Using Counting Region" << rcb;
    ModelItem::Arguments args;
    args["Regions"] = rcb;
    countingRegionExt->initialize(args);
  }

  m_model->addFilter(m_filter);
  m_model->addSegmentation(m_segs);

  m_model->addRelation(m_channel, m_filter, Channel::LINK);
  foreach(Segmentation *seg, m_segs)
  {
    m_model->addRelation(m_filter, seg, CREATELINK);
    m_model->addRelation(m_sample, seg, "where");
    m_model->addRelation(m_channel, seg, Channel::LINK);
    seg->initializeExtensions();
  }
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::undo()
{
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  foreach(Segmentation *seg, m_segs)
  {
    m_model->removeRelation(m_filter, seg, CREATELINK);
    m_model->removeRelation(m_sample, seg, "where");
    m_model->removeRelation(m_channel, seg, Channel::LINK);
  }

  m_model->removeSegmentation(m_segs);
  m_model->removeFilter(m_filter);
}

static const QString SEGMHA = "segmha";

//-----------------------------------------------------------------------------
SegmhaImporter::SegmhaImporter()
: m_model(NULL)
, m_undoStack(NULL)
, m_viewManager(NULL)
{
}

//-----------------------------------------------------------------------------
void SegmhaImporter::initFilterFactory(EspinaFactory* factory)
{
  factory->registerFilter(SegmhaImporterFilter::TYPE, this);
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
void SegmhaImporter::initReaderFactory(EspinaModel* model,
                                       QUndoStack *undoStack,
                                       ViewManager *vm)
{
  m_model = model;
  m_undoStack = undoStack;
  m_viewManager = vm;
  // Register filter and reader factories
  QStringList supportedExtensions;
  supportedExtensions << SEGMHA;
  m_model->factory()->registerReaderFactory(this,
                                            SegmhaImporterFilter::SUPPORTED_FILES,
                                            supportedExtensions);
}

//-----------------------------------------------------------------------------
bool SegmhaImporter::readFile(const QFileInfo file)
{
  qDebug() << file.absoluteFilePath();
  Q_ASSERT(SEGMHA == file.suffix());

  m_undoStack->beginMacro("Import Segmha");

  QFileDialog fileDialog;
  fileDialog.setObjectName("SelectChannelFile");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setWindowTitle(QString("Select channel file for %1:").arg(file.fileName()));
  fileDialog.setDirectory(file.dir());
  fileDialog.setFilter(CHANNEL_FILES);

  if (fileDialog.exec() != QDialog::Accepted)
    return false;

  Channel *channel;
  if (EspinaIO::SUCCESS != EspinaIO::loadChannel(fileDialog.selectedFiles().first(), m_model, m_undoStack, &channel))
    return false;

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
  Sample *sample = channel->sample();
  m_undoStack->push(new UndoCommand(sample, channel, filter, m_model));
  m_undoStack->endMacro();
  QApplication::restoreOverrideCursor();

  return true;
}

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, SegmhaImporter)