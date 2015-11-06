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
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Model/Utils/ModelUtils.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

//-----------------------------------------------------------------------------
PipelineSources::PipelineSources(GUI::View::ViewState &viewState)
: m_viewState(viewState)
{
  connect(&m_viewState, SIGNAL(representationsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
          this,         SLOT(onRepresentationsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));

  connect(&m_viewState, SIGNAL(representationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)),
          this,         SLOT(onRepresentationColorsInvalidated(ViewItemAdapterList,GUI::Representations::FrameCSPtr)));
}

//-----------------------------------------------------------------------------
PipelineSources::~PipelineSources()
{
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::sources(const ItemAdapter::Type &type) const
{
  return ItemAdapter::Type::CHANNEL == type?m_stacks:m_segmentations;
}

//-----------------------------------------------------------------------------
bool PipelineSources::isEmpty() const
{
  return m_stacks.isEmpty() && m_segmentations.isEmpty();
}

//-----------------------------------------------------------------------------
int PipelineSources::size() const
{
  return m_stacks.size() + m_segmentations.size();
}

//-----------------------------------------------------------------------------
void PipelineSources::insert(ViewItemAdapterList sources)
{
  ViewItemAdapterList stacks;
  ViewItemAdapterList segmentations;

  for (auto source : sources)
  {
    Q_ASSERT(!contains(source));

    connect(source,       SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
            &m_viewState, SLOT(invalidateRepresentations(ViewItemAdapterPtr)));

    if (isChannel(source))
    {
      stacks << source;
      m_stacks << source;
    }
    else
    {
      Q_ASSERT(isSegmentation(source));
      segmentations << source;
      m_segmentations << source;
    }
  }

  if (!stacks.isEmpty() || !segmentations.isEmpty())
  {
    auto frame = m_viewState.invalidateRepresentations(sources);

    if (!stacks.isEmpty())
    {
      emit stacksAdded(stacks, frame);
    }

    if (!segmentations.isEmpty())
    {
      emit segmentationsAdded(segmentations, frame);
    }
  }
}

//-----------------------------------------------------------------------------
bool PipelineSources::contains(ViewItemAdapterPtr source) const
{
  return isChannel(source)?m_stacks.contains(channelPtr(source))
                          :m_segmentations.contains(segmentationPtr(source));
}

//-----------------------------------------------------------------------------
void PipelineSources::remove(ViewItemAdapterList sources)
{
  ViewItemAdapterList stacks;
  ViewItemAdapterList segmentations;

  for (auto source : sources)
  {
    Q_ASSERT(contains(source));

    disconnect(source,       SIGNAL(representationsInvalidated(ViewItemAdapterPtr)),
               &m_viewState, SLOT(invalidateRepresentations(ViewItemAdapterPtr)));


    if (isChannel(source))
    {
      stacks << source;
      m_stacks.removeOne(source);
    }
    else
    {
      Q_ASSERT(isSegmentation(source));
      segmentations << source;
      m_segmentations.removeOne(source);
    }
  }

  if (!stacks.isEmpty() || !segmentations.isEmpty())
  {
    auto frame = m_viewState.invalidateRepresentations(sources);

    if (!stacks.isEmpty())
    {
      emit stacksRemoved(stacks, frame);
    }

    if (!segmentations.isEmpty())
    {
      emit segmentationsRemoved(segmentations, frame);
    }
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto invalidated = classifyViewItems(items);

  if (!invalidated.stacks.isEmpty())
  {
    emit stacksInvalidated(invalidated.stacks, frame);
  }
  if (!invalidated.segmentations.isEmpty())
  {
    emit segmentationsInvalidated(invalidated.segmentations, frame);
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame)
{
  auto invalidated = classifyViewItems(items);

  if (!invalidated.stacks.isEmpty())
  {
    emit stackColorsInvalidated(invalidated.stacks, frame);
  }
  if (!invalidated.segmentations.isEmpty())
  {
    emit segmentationColorsInvalidated(invalidated.segmentations, frame);
  }
}
