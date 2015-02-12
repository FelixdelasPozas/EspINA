/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "PipelineSources.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
PipelineSources::~PipelineSources()
{
  qDebug() << "Destroy";
}

//-----------------------------------------------------------------------------
void PipelineSources::addSource(ViewItemAdapterPtr source)
{
  Q_ASSERT(!contains(source));

  source->output()->update();

  insert(source);

  //NOTE: We could propagate it using the Qt Model instead
  connect(source, SIGNAL(outputChanged(ViewItemAdapterPtr)),
          this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr)));

  emit sourceAdded(source);
}

//-----------------------------------------------------------------------------
void PipelineSources::onSourceUpdated(ViewItemAdapterPtr source)
{
  if (contains(source))
  {
    emit sourceUpdated(source);
  }
  else
  {
    qWarning() << "Update Source on non-registered source";
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::removeSource(ViewItemAdapterPtr source)
{
  Q_ASSERT(contains(source));

  disconnect(source, SIGNAL(outputChanged(ViewItemAdapterPtr)),
             this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr)));

  remove(source);

  emit sourceRemoved(source);
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::sources() const
{
  return m_sources;
}

//-----------------------------------------------------------------------------
void PipelineSources::onSourceAdded(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time) const
{
  if(source == this)
  {
    emit sourceAdded(item, time);
  }
  else
  {
    emit updateTimeStamp(time);
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onSourceUpdated(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time) const
{
  if(source == this)
  {
    emit sourceUpdated(item, time);
  }
  else
  {
    emit updateTimeStamp(time);
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onSourceRemoved(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time) const
{
  if(source == this)
  {
    emit sourceRemoved(item, time);
  }
  else
  {
    emit updateTimeStamp(time);
  }
}

