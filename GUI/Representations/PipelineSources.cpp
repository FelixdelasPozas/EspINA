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

  emit sourcesAdded(createList(source));
}

//-----------------------------------------------------------------------------
void PipelineSources::onSourceUpdated(ViewItemAdapterPtr source)
{
  if (contains(source))
  {
    emit sourcesUpdated(createList(source));
  }
  else
  {
    qWarning() << "Updating Source on non-registered source";
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::removeSource(ViewItemAdapterPtr source)
{
  remove(source);

  emit sourcesRemoved(createList(source));
}

//-----------------------------------------------------------------------------
void PipelineSources::clear()
{
  ViewItemAdapterList sources = m_sources;

  for (auto source : m_sources)
  {
    remove(source);
  }

  emit sourcesRemoved(sources);
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::sources() const
{
  return m_sources;
}

//-----------------------------------------------------------------------------
void PipelineSources::insert(ViewItemAdapterPtr source)
{
  //NOTE: We could propagate it using the Qt Model instead
//   connect(source, SIGNAL(outputChanged(ViewItemAdapterPtr)),
//           this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr)));

  m_sources.append(source);
}

//-----------------------------------------------------------------------------
void PipelineSources::remove(ViewItemAdapterPtr source)
{
  Q_ASSERT(contains(source));

//   disconnect(source, SIGNAL(outputChanged(ViewItemAdapterPtr)),
//              this,   SIGNAL(sourceUpdated(ViewItemAdapterPtr)));

  m_sources.removeOne(source);
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::createList(ViewItemAdapterPtr item) const
{
  ViewItemAdapterList list;

  list << item;

  return list;
}
