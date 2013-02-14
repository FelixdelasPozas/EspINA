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

#include <Core/IO/EspinaIO.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Relations.h>
#include <Undo/AddChannel.h>
#include <Undo/AddRelation.h>
#include <Undo/AddSample.h>

#include <Plugins/CountingFrame/Extensions/CountingFrameExtension.h>

#include <QApplication>
#include <QFileDialog>
#include <QDebug>

using namespace EspINA;

const QString INPUTLINK = "Input";

const QString SegmhaImporter::UndoCommand::FILTER_TYPE = "Segmha Importer";

//-----------------------------------------------------------------------------
SegmhaImporter::UndoCommand::UndoCommand(SampleSPtr               sample,
                                         ChannelSPtr              channel,
                                         SegmhaImporterFilterSPtr filter,
                                         EspinaModel              *model,
                                         QUndoCommand             *parent)
: QUndoCommand(parent)
, m_model(model)
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
    SegmentationSPtr seg;
    foreach(Filter::Output output, m_filter->outputs())
    {
      seg = m_model->factory()->createSegmentation(m_filter, output.id);
      m_filter->initSegmentation(seg, output.id);
      m_segs << seg;
    }
  }

  Channel::ExtensionPtr cfExtension = m_channel->extension(CountingFrameExtensionID);
  if (!cfExtension)
  {
    Channel::ExtensionPtr prototype = m_model->factory()->channelExtension(CountingFrameExtensionID);
    if (prototype)
    {
      cfExtension = prototype->clone();
      m_channel->addExtension(cfExtension);
    }
  }

 if (cfExtension)
  {
    Nm inclusive[3], exclusive[3];
    m_filter->countingFrame(inclusive, exclusive);
    double spacing[3];
    m_channel->volume()->spacing(spacing);
    for(int i=0; i<3;i++)
    {
      inclusive[i] = inclusive[i]*spacing[i];
      exclusive[i] = exclusive[i]*spacing[i];
    }

    QString rcb = QString("RectangularCountingFrame=%1,%2,%3,%4,%5,%6;")
      .arg(inclusive[0]).arg(inclusive[1]).arg(inclusive[2])
      .arg(exclusive[0]).arg(exclusive[1]).arg(exclusive[2]);
    //qDebug() << "Using Counting Frame" << rcb;
    ModelItem::Arguments args;
    args["CountingFrameExtension"] = "CFs=[" + rcb + "];";
    cfExtension->initialize(args);
  }

  m_model->addFilter(m_filter);
  m_model->addSegmentation(m_segs);

  m_model->addRelation(m_channel, m_filter, Channel::LINK);
  foreach(SegmentationSPtr seg, m_segs)
  {
    m_model->addRelation(m_filter,  seg, Filter::CREATELINK);
    m_model->addRelation(m_sample,  seg, Relations::LOCATION);
    m_model->addRelation(m_channel, seg, Channel::LINK);
    seg->initializeExtensions();
  }
}

//-----------------------------------------------------------------------------
void SegmhaImporter::UndoCommand::undo()
{
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  foreach(SegmentationSPtr seg, m_segs)
  {
    m_model->removeRelation(m_filter,  seg, Filter::CREATELINK);
    m_model->removeRelation(m_sample,  seg, Relations::LOCATION);
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
SegmhaImporter::~SegmhaImporter()
{
  qDebug() << "********************************************************";
  qDebug() << "              Destroying SegmhaImporter Plugin";
  qDebug() << "********************************************************";
}

//-----------------------------------------------------------------------------
void SegmhaImporter::initFactoryExtension(EspinaFactory *factory)
{
  factory->registerFilter(this, UndoCommand::FILTER_TYPE);
}

//-----------------------------------------------------------------------------
FilterSPtr SegmhaImporter::createFilter(const QString              &filter,
                                        const Filter::NamedInputs  &inputs,
                                        const ModelItem::Arguments &args)
{
  Q_ASSERT(UndoCommand::FILTER_TYPE == filter);
  return FilterSPtr(new SegmhaImporterFilter(inputs, args, UndoCommand::FILTER_TYPE));
}

//-----------------------------------------------------------------------------
void SegmhaImporter::initFileReader(EspinaModel *model,
                                    QUndoStack  *undoStack,
                                    ViewManager *viewManager)
{
  m_model = model;
  m_undoStack = undoStack;
  m_viewManager = viewManager;
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

  QApplication::setOverrideCursor(Qt::ArrowCursor);
  QFileDialog fileDialog;
  fileDialog.setObjectName("SelectChannelFile");
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setWindowTitle(QString("Select channel file for %1:").arg(file.fileName()));
  fileDialog.setDirectory(file.dir());
  fileDialog.setFilter(CHANNEL_FILES);

  int res = fileDialog.exec();
  QApplication::restoreOverrideCursor();

  if (QDialog::Accepted != res)
    return false;

  // TODO 2012-12-29: Como gestionar las dependecias? dentro del undo command?
  ChannelSPtr channel;
  if (EspinaIO::SUCCESS != EspinaIO::loadChannel(fileDialog.selectedFiles().first(), m_model, channel))
    return false;

  Filter::NamedInputs inputs;
  Filter::Arguments args;
  args[SegmhaImporterFilter::FILE] = file.absoluteFilePath();
  SegmhaImporterFilter::Parameters params(args);
  params.setSpacing(channel->volume()->toITK()->GetSpacing());

  QApplication::setOverrideCursor(Qt::WaitCursor);
  SegmhaImporterFilterSPtr filter(new SegmhaImporterFilter(inputs, args, UndoCommand::FILTER_TYPE));
  filter->update();
  if (filter->outputs().isEmpty())
    return false;

  SampleSPtr sample = channel->sample();
  m_undoStack->push(new UndoCommand(sample, channel, filter, m_model));
  m_undoStack->endMacro();
  QApplication::restoreOverrideCursor();

  return true;
}

Q_EXPORT_PLUGIN2(SegmhaImporterPlugin, EspINA::SegmhaImporter)
