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


#include "CountingFrameChannelExtension.h"

#include "CountingFramePanel.h"
#include "CountingFrameSegmentationExtension.h"
#include "CountingFrames/CountingFrame.h"
#include "CountingFrames/RectangularCountingFrame.h"
#include "CountingFrames/AdaptiveCountingFrame.h"

#include <Core/Model/Sample.h>
#include <Core/Model/Channel.h>
#include <Core/Extensions/Margins/MarginsSegmentationExtension.h>
#include <GUI/ViewManager.h>

#include <QDebug>
#include <QApplication>

typedef ModelItem::ArgumentId ArgumentId;

const ModelItemExtension::ExtId CountingFrameChannelExtension::ID       = "CountingFrameExtension";
const ModelItemExtension::ExtId CountingFrameChannelExtension::ID_1_2_5 = "CountingRegionExtension";

const ArgumentId COUNTING_FRAMES_1_2_5 = "Regions"; // Backwards compatibility versions < 1.2.5
const ArgumentId CountingFrameChannelExtension::COUNTING_FRAMES = "CFs";

//-----------------------------------------------------------------------------
CountingFrameChannelExtension::CountingFrameChannelExtension(CountingFramePanel* plugin,
                                                               ViewManager *vm)
: m_plugin(plugin)
, m_viewManager(vm)
{

}

//-----------------------------------------------------------------------------
CountingFrameChannelExtension::~CountingFrameChannelExtension()
{

}

//-----------------------------------------------------------------------------
void CountingFrameChannelExtension::initialize(ModelItem::Arguments args)
{
  ModelItem::Arguments extArgs(args.value(ID, QString()));

  if (extArgs.isEmpty())
    extArgs = ModelItem::Arguments(args.value(ID_1_2_5, QString()));

  QStringList countingFrames = extArgs.value(COUNTING_FRAMES, "").split(";", QString::SkipEmptyParts);

  if (countingFrames.isEmpty()) // Check previous tag
    countingFrames = extArgs.value(COUNTING_FRAMES_1_2_5, "").split(";", QString::SkipEmptyParts);

  foreach (QString countingFrame, countingFrames)
  {
    QString type = countingFrame.section('=',0,0);
    QStringList margins = countingFrame.section('=',-1).split(',');
    Nm inclusion[3], exclusion[3];
    for (int i=0; i<3; i++)
    {
      inclusion[i] = margins[i].toDouble();
      exclusion[i] = margins[3+i].toDouble();
    }
    if (RectangularCountingFrame::ID == type
      ||RectangularCountingFrame::ID_1_2_5 == type)
      m_plugin->createRectangularCF(m_channel, inclusion, exclusion);
    else if (AdaptiveCountingFrame::ID == type
      || AdaptiveCountingFrame::ID_1_2_5 == type)
      m_plugin->createAdaptiveCF(m_channel, inclusion, exclusion);
  }

  m_args = extArgs;

  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
QString CountingFrameChannelExtension::serialize() const
{
  QStringList cfValue;
  foreach(CountingFrame *countingFrame, m_countingFrames)
    cfValue << countingFrame->serialize();

  m_args[COUNTING_FRAMES] = "[" + cfValue.join(";") + "]";
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtIdList CountingFrameChannelExtension::dependencies() const
{
  ExtIdList deps;
  deps << MarginsSegmentationExtension::ID;
  return deps;
}

//-----------------------------------------------------------------------------
ChannelExtension* CountingFrameChannelExtension::clone()
{
  return new CountingFrameChannelExtension(m_plugin, m_viewManager);
}

//-----------------------------------------------------------------------------
void CountingFrameChannelExtension::addCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(!m_countingFrames.contains(countingFrame));
  m_countingFrames << countingFrame;

  Sample *sample = m_channel->sample();
  Q_ASSERT(sample);
  ModelItem::Vector items = sample->relatedItems(ModelItem::OUT, "where");
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      ModelItemExtension *ext = item->extension(CountingFrameSegmentationExtension::ID);
      Q_ASSERT(ext);
      CountingFrameSegmentationExtension *segExt = dynamic_cast<CountingFrameSegmentationExtension *>(ext);
      segExt->setCountingFrames(m_countingFrames);
    }
  }
  connect(countingFrame, SIGNAL(modified(CountingFrame*)),
          this, SLOT(countinfFrameUpdated(CountingFrame*)));
  //m_viewManager->updateSegmentationRepresentations();
  //m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void CountingFrameChannelExtension::deleteCountingFrame(CountingFrame* countingFrame)
{
  Q_ASSERT(m_countingFrames.contains(countingFrame));
  m_countingFrames.removeOne(countingFrame);

  Sample *sample = m_channel->sample();
  Q_ASSERT(sample);
  ModelItem::Vector items = sample->relatedItems(ModelItem::OUT, Sample::WHERE);
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      ModelItemExtension *ext = item->extension(CountingFrameSegmentationExtension::ID);
      Q_ASSERT(ext);
      CountingFrameSegmentationExtension *segExt = dynamic_cast<CountingFrameSegmentationExtension *>(ext);
      segExt->setCountingFrames(m_countingFrames);
    }
  }
  //m_viewManager->updateSegmentationRepresentations();
  //m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void CountingFrameChannelExtension::countinfFrameUpdated(CountingFrame* countingFrame)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Sample *sample = m_channel->sample();
  Q_ASSERT(sample);
  ModelItem::Vector items = sample->relatedItems(ModelItem::OUT, Sample::WHERE);
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      ModelItemExtension *ext = item->extension(CountingFrameSegmentationExtension::ID);
      Q_ASSERT(ext);
      CountingFrameSegmentationExtension *segExt = dynamic_cast<CountingFrameSegmentationExtension *>(ext);
      segExt->evaluateCountingFrame(countingFrame);
    }
  }
  QApplication::restoreOverrideCursor();
}
