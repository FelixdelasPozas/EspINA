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

#include "PipelineSourcesFilter.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
PipelineSourcesFilter::PipelineSourcesFilter(ModelAdapterSPtr model, const ItemAdapter::Type type)
: m_model{model}
, m_type{type}
{
  connect(m_model.get(), SIGNAL(viewItemsAdded(ViewItemAdapterSList,TimeStamp)),
          this,          SLOT(onSourcesAdded(ViewItemAdapterSList,TimeStamp)));
  connect(m_model.get(), SIGNAL(viewItemsAboutToBeRemoved(ViewItemAdapterSList,TimeStamp)),
          this,          SLOT(onSourcesRemoved(ViewItemAdapterSList,TimeStamp)));

  connect(m_model.get(), SIGNAL(representationsModified(ViewItemAdapterSList,TimeStamp)),
          this,          SLOT(onRepresentationModified(ViewItemAdapterSList,TimeStamp)));
  connect(m_model.get(), SIGNAL(aboutToBeReset(TimeStamp)),
          this,          SLOT(onReset(TimeStamp)));
}

//-----------------------------------------------------------------------------
PipelineSourcesFilter::~PipelineSourcesFilter()
{
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::setSelectedSources(ViewItemAdapterSList sources)
{
  m_sources.clear();
  m_selectedSources.clear();

  for (auto source : sources)
  {
    m_sources         << source.get();
    m_selectedSources << source.get();
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onSourcesAdded(ViewItemAdapterSList sources, TimeStamp t)
{
  auto filteredSources = filter(sources);

  insert(filteredSources);

  if (filteredSources.isEmpty())
  {
    emit updateTimeStamp(t);
  }
  else
  {
    emit sourcesAdded(filteredSources, t);
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onSourcesRemoved(ViewItemAdapterSList sources, TimeStamp t)
{
  auto filteredSources = filter(sources);

  remove(filteredSources);

  if (filteredSources.isEmpty())
  {
    emit updateTimeStamp(t);
  }
  else
  {
    emit sourcesRemoved(filteredSources, t);
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onRepresentationModified(ViewItemAdapterSList sources, TimeStamp t)
{
  auto filteredSources = filter(sources);

  if (filteredSources.isEmpty())
  {
    emit updateTimeStamp(t);
  }
  else
  {
    emit representationsModified(filteredSources, t);
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onReset(TimeStamp t)
{
  if (!m_sources.isEmpty())
  {
    emit sourcesRemoved(m_sources, t);

    m_sources.clear();
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSourcesFilter::filter(ViewItemAdapterSList sources)
{
  ViewItemAdapterList result;

  for (auto source : sources)
  {
    if (acceptedType(source) && acceptedSource(source))
    {
      result << source.get();
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
bool PipelineSourcesFilter::acceptedType(ViewItemAdapterSPtr source) const
{
  return source->type() == m_type;
}

//-----------------------------------------------------------------------------
bool PipelineSourcesFilter::acceptedSource(ViewItemAdapterSPtr source) const
{
  return m_selectedSources.isEmpty() || m_selectedSources.contains(source.get());
}
