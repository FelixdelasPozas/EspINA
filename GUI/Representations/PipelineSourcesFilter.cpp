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
PipelineSourcesFilter::PipelineSourcesFilter(ModelAdapterSPtr model, const ItemAdapter::Type type, GUI::View::ViewState &viewState)
: PipelineSources(viewState)
, m_model{model}
, m_type{type}
{
  connect(m_model.get(), SIGNAL(viewItemsAdded(ViewItemAdapterSList)),
          this,          SLOT(onSourcesAdded(ViewItemAdapterSList)));

  connect(m_model.get(), SIGNAL(viewItemsAboutToBeRemoved(ViewItemAdapterSList)),
          this,          SLOT(onSourcesRemoved(ViewItemAdapterSList)));

  connect(m_model.get(), SIGNAL(aboutToBeReset()),
          this,          SLOT(onReset()));
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
void PipelineSourcesFilter::onSourcesAdded(ViewItemAdapterSList sources)
{
  auto filteredSources = filter(sources);

  insert(filteredSources);

  auto frame = createFrame();

  if (filteredSources.isEmpty())
  {
    emit updateTimeStamp(frame);
  }
  else
  {
    emit sourcesAdded(filteredSources, frame);
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onSourcesRemoved(ViewItemAdapterSList sources)
{
  auto filteredSources = filter(sources);

  remove(filteredSources);

  auto frame = createFrame();

  if (filteredSources.isEmpty())
  {
    emit updateTimeStamp(frame);
  }
  else
  {
    emit sourcesRemoved(filteredSources, frame);
  }
}

//-----------------------------------------------------------------------------
void PipelineSourcesFilter::onReset()
{
  if (!m_sources.isEmpty())
  {
    emit sourcesRemoved(m_sources, createFrame());

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