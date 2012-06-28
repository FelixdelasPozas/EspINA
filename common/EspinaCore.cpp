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


#include "EspinaCore.h"

#include "model/EspinaModel.h"

#include "model/EspinaFactory.h"
#include "model/Channel.h"
#include "model/ChannelReader.h"
#include "undo/AddSample.h"
#include "undo/AddChannel.h"
#include "undo/AddRelation.h"
#include "IO/FilePack.h"

#include <QUndoStack>
#include <QColorDialog>
#include <QApplication>

EspinaCore *EspinaCore::m_singleton = NULL;

//------------------------------------------------------------------------
EspinaCore::EspinaCore()
: m_activeTaxonomy(NULL)
, m_sample        (NULL)
, m_model         (new EspinaModel())
, m_undoStack     (new QUndoStack())
, m_viewManager   (new ViewManager())
{
}

//------------------------------------------------------------------------
EspinaCore* EspinaCore::instance()
{
  if (!m_singleton)
    m_singleton = new EspinaCore();

  return m_singleton;
}

//------------------------------------------------------------------------
void EspinaCore::setActiveTaxonomy(TaxonomyNode* tax)
{
  m_activeTaxonomy = tax;
}

//------------------------------------------------------------------------
bool EspinaCore::loadFile(const QFileInfo file)
{
  bool status = false;
  const QString ext = file.completeSuffix();//:::fileName() extension(file);
  if ("mha" == ext || "mhd" == ext || "tiff" == ext || "tif" == ext)
  {
    status = loadChannel(file);
  } else if ("seg" == ext)
  {
    status = IOEspinaFile::loadFile(file,
                                    EspinaCore::instance()->model());
  }else
    status = EspinaFactory::instance()->readFile(file.absoluteFilePath(), ext);

  return status;
}

//------------------------------------------------------------------------
bool EspinaCore::loadChannel(const QFileInfo file)
{
  // Try to recover sample form DB using channel information
  Sample *existingSample = EspinaCore::instance()->sample();

  EspinaFactory *factory = EspinaFactory::instance();

  if (!existingSample)
  {
    // TODO: Look for real channel's sample in DB or prompt dialog
    // Try to recover sample form DB using channel information
    Sample *sample = factory->createSample(file.baseName());
    EspinaCore::instance()->setSample(sample);

    m_undoStack->push(new AddSample(sample));
    existingSample = sample;
  }

  //TODO: Check for channel information in DB
  QColor stainColor;
  QColorDialog stainColorSelector;
  stainColorSelector.setWindowTitle("Select Stain Color");
  if (stainColorSelector.exec() == QDialog::Accepted)
    stainColor = stainColorSelector.selectedColor();
  else
    stainColor = QColor(Qt::black);

  QApplication::setOverrideCursor(Qt::WaitCursor);

  Filter::NamedInputs noInputs;
  Filter::Arguments readerArgs;
  readerArgs[ChannelReader::FILE] = file.absoluteFilePath();
  ChannelReader *reader = new ChannelReader(noInputs, readerArgs);
  reader->update();

  Channel::CArguments args;
  args[Channel::ID] = file.fileName();
  args.setColor(stainColor.hueF());
  Channel *channel = factory->createChannel(reader, 0);
  channel->initialize(args);
    //file.absoluteFilePath(), args);

  double pos[3];
  existingSample->position(pos);
  channel->setPosition(pos);

  m_undoStack->beginMacro("Add Data To Analysis");
  m_undoStack->push(new AddChannel(reader, channel));
  m_undoStack->push(new AddRelation(existingSample, channel, Channel::STAINLINK));
  existingSample->initialize();
  channel->initialize();

  m_undoStack->endMacro();
  QApplication::restoreOverrideCursor();

  return true;
}

//------------------------------------------------------------------------
void EspinaCore::closeCurrentAnalysis()
{
  emit currentAnalysisClosed();
  m_activeTaxonomy = NULL;
  m_sample = NULL;
  m_model->reset();
  m_undoStack->clear();
  Filter::resetId();
}
