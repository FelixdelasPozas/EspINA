/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// ESPINA
#include "PipelineSources.h"
#include <GUI/View/ViewState.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
PipelineSources::PipelineSources(GUI::View::ViewState &viewState)
: m_viewState(viewState)
{
  connect(&m_viewState, SIGNAL(representationsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
          this,                         SLOT(onRepresentationsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));

  connect(&m_viewState, SIGNAL(representationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
          this,                         SLOT(onRepresentationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
}

//-----------------------------------------------------------------------------
PipelineSources::~PipelineSources()
{
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::sources() const
{
  return m_sources;
}

//-----------------------------------------------------------------------------
void PipelineSources::insert(ViewItemAdapterList sources)
{
  for (auto source : sources)
  {
    Q_ASSERT(!contains(source));

    connect(source,       SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
            &m_viewState, SLOT(invalidateRepresentations(ViewItemAdapterPtr)));

    m_sources.append(source);
  }
}

//-----------------------------------------------------------------------------
bool PipelineSources::contains(ViewItemAdapterPtr source) const
{
  return m_sources.contains(source);
}

//-----------------------------------------------------------------------------
void PipelineSources::remove(ViewItemAdapterList sources)
{
  for (auto source : sources)
  {
    Q_ASSERT(contains(source));

    disconnect(source,       SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
               &m_viewState, SLOT(invalidateRepresentations(ViewItemAdapterPtr)));

    m_sources.removeOne(source);
  }
}

//-----------------------------------------------------------------------------
GUI::Representations::FrameCSPtr PipelineSources::createFrame() const
{
  return m_viewState.createFrame();
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto invalidatedItems = acceptedItems(items);

  if (!invalidatedItems.isEmpty())
  {
    emit representationsInvalidated(invalidatedItems, frame);
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto invalidatedItems = acceptedItems(items);

  if (!invalidatedItems.isEmpty())
  {
    emit representationColorsInvalidated(invalidatedItems, frame);
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::acceptedItems(const ViewItemAdapterList& items)
{
  ViewItemAdapterList acceptedItems;

  for (auto item : items)
  {
    if (contains(item))
    {
      acceptedItems << item;
    }
  }

  return acceptedItems;
}
