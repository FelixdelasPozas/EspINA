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

#include <QUndoStack>
#include "model/EspinaFactory.h"
#include "model/Channel.h"
#include <QColorDialog>
#include <QApplication>
#include "cache/CachedObjectBuilder.h"
#include "undo/AddSample.h"
#include "undo/AddChannel.h"
#include "undo/AddRelation.h"
#include "File.h"


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
bool EspinaCore::loadFile(const QString file)
{
  bool status = false;
  const QString ext = File::extension(file);
  if (ext == "pvd" || ext == "mha" || ext == "mhd")
  {
    status = loadChannel(file);
  }else
    status = EspinaFactory::instance()->readFile(file, ext);

  return status;
}

//------------------------------------------------------------------------
bool EspinaCore::loadChannel(const QString file)
{
  // Try to recover sample form DB using channel information
  Sample *existingSample = EspinaCore::instance()->sample();

  if (!existingSample)
  {
    File channelFile(file);
    // TODO: Check for channel sample
    const QString SampleName  = channelFile.name();
    const QString channelName = channelFile.extendedName(file);

    // Try to recover sample form DB using channel information
    Sample *sample = EspinaFactory::instance()->createSample(SampleName);
    EspinaCore::instance()->setSample(sample);

    m_undoStack->push(new AddSample(sample));
    existingSample = sample;
  }

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqFilter *channelReader = cob->loadFile(file);
  Q_ASSERT(channelReader->getNumberOfData() == 1);


  Channel::CArguments args;

  pqData channelData(channelReader, 0);
  args[Channel::ID] = channelData.id();

  //TODO: Check for channel information in DB
  QColorDialog dyeSelector;
  if (dyeSelector.exec() == QDialog::Accepted)
  {
    args.setColor(dyeSelector.selectedColor().hueF());
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  Channel *channel = EspinaFactory::instance()->createChannel(file, args);

  double pos[3];
  existingSample->position(pos);
  channel->setPosition(pos);

  m_undoStack->beginMacro("Add Data To Analysis");
  m_undoStack->push(new AddChannel(channel));
  m_undoStack->push(new AddRelation(existingSample, channel, "mark"));//TODO: como se llama esto???
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
}
